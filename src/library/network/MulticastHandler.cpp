#include "MulticastHandler.hpp"

namespace Lecista {

MulticastHandler::MulticastHandler(IOHandler& io, HashDatabase& db) : m_db(db)
{
	m_network = new MulticastNetwork(io,
		boost::bind(&MulticastHandler::dispatch, this, _1, _2, _3, _4, _5));
	m_gateway = new MulticastGateway(m_network);
	m_election = new GatewayElection(
		m_network,
		boost::bind(&MulticastGateway::update, m_gateway, _1, _2));

	m_gateway->onTimeout(boost::bind(&GatewayElection::gatewayTimeout, m_election));
}

MulticastHandler::~MulticastHandler()
{
	delete m_election;
	delete m_gateway;
	delete m_network;
}

void MulticastHandler::dispatch(
	boost::asio::ip::address senderAddress,
	MulticastNetwork::Command command,
	char *args,
	unsigned char argsSize,
	bool forward)
{
	typedef MulticastNetwork::Command Command;

	m_senderAddress = &senderAddress;
	bool valid = false;

	if (Command::Candidate == command && sizeof(uint32_t) == argsSize) {
		valid = true;
		on_candidate(ntohl(*reinterpret_cast<uint32_t*>(args)));
	}
	else if (Command::ElectGateway == command && 0 == argsSize) {
		valid = true;
		on_electGateway();
	}
	else if (Command::Forward == command && 0 < argsSize)
	{
		valid = true;
		on_forward(static_cast<Command>(args[0]), args + 1, argsSize - 1);
	}
	else if (Command::Gateway == command && 0 == argsSize)
	{
		valid = true;
		on_gateway();
	}
	else if (Command::Hello == command && sizeof(float) < argsSize) {
		valid = true;
		uint32_t b = ntohl(*reinterpret_cast<uint32_t*>(args + argsSize - sizeof(float)));
		on_hello(std::string(args, argsSize - sizeof(float)),
			*reinterpret_cast<float*>(&b));
	}
	else if (Command::Message == command && 0 < argsSize) {
		valid = true;
		on_message(std::string(args, argsSize));
	}
	else if (Command::RemoteGateway == command && 0 == argsSize) {
		valid = true;
		on_remoteGateway();
	}
	else if (Command::SearchBlock == command && sizeof(uint32_t) < argsSize) {
		valid = true;
		on_searchBlock(std::string(args, argsSize - sizeof(uint32_t)),
			*reinterpret_cast<uint32_t*>(args + argsSize - sizeof(uint32_t)));
	}
	else if (Command::SearchFileName == command && 0 < argsSize) {
		valid = true;
		on_searchFileName(std::string(args, argsSize));
	}
	else if (Command::SearchFileNameReply == command && 0 < argsSize) {
		valid = true;
		std::map<std::string, std::pair<float, std::string>> results;

		int index = 0;
		while (index < argsSize) {
			float filesize;
			int stringLength = 0;
			for (int i = index + Hash::SIZE + sizeof filesize; i < argsSize; i++) {
				if (args[i] == '\0') {
					break;
				}

				++stringLength;
			}

			if (index + stringLength + Hash::SIZE + sizeof filesize >= argsSize) {
				break;
			}

			std::string hash(args + index, Hash::SIZE);
			filesize = *reinterpret_cast<float*>(args + index + Hash::SIZE);

			std::string filename(args + index + Hash::SIZE + sizeof filesize, stringLength);
			results[filename] = std::pair<float, std::string>(filesize, hash);
			index += Hash::SIZE + sizeof filesize + stringLength + 1;
		}

		on_searchFileNameReply(results);
	}

	if (valid && forward) {
		m_gateway->forward(*m_senderAddress, command, args, argsSize);
	}
}

/* Message handling methods below */

void MulticastHandler::on_candidate(uint32_t id)
{
	LOG_DEBUG("on_candidate(" << id << ") from " << m_senderAddress->to_string());
	m_election->registerCandidate(*m_senderAddress, id);
}

void MulticastHandler::on_electGateway()
{
	LOG_DEBUG("on_electGateway() from " << m_senderAddress->to_string());
	m_election->start();
}

void MulticastHandler::on_forward(MulticastNetwork::Command command, char* args, unsigned char argsSize)
{
	LOG_DEBUG("on_forward() from " << m_senderAddress->to_string());
	m_gateway->on_forward(*m_senderAddress, command, args, argsSize);
}

void MulticastHandler::on_gateway()
{
	//LOG_DEBUG("on_gateway() from " << m_senderAddress->to_string());
	m_gateway->on_gateway(*m_senderAddress);
}

void MulticastHandler::on_hello(std::string name, float sharedSize)
{
	LOG_DEBUG("on_hello(" << name << ", " << sharedSize << ") from " << m_senderAddress->to_string());
}

void MulticastHandler::on_message(std::string message)
{
	LOG_DEBUG("on_message(" << message << ") from " << m_senderAddress->to_string());
}

void MulticastHandler::on_remoteGateway()
{
	//LOG_DEBUG("on_remoteGateway() from " << m_senderAddress->to_string());
	m_gateway->on_remoteGateway(*m_senderAddress);
}

void MulticastHandler::on_searchBlock(std::string rootHash, uint32_t blockId)
{
	LOG_DEBUG("on_searchBlock(" << rootHash << ", " << blockId << ") from " << m_senderAddress->to_string());
}

void MulticastHandler::on_searchFileName(std::string filename)
{
	LOG_DEBUG("on_searchFileName(" << filename << ") from " << m_senderAddress->to_string());

	boost::shared_ptr<std::deque<HashDatabase::FileEntry::SharedPtr>> m = m_db.search(filename);
	std::string answer;
	int length = 0;
	for (auto i = m->begin(); i != m->end(); ++i) {
		std::string match((*i)->file()->rootHash()->data(), 20);
		float filesize = static_cast<float>((*i)->file()->filesize());
		char size[sizeof(filesize)];
		std::memcpy(&size, &filesize, sizeof filesize);
		match += std::string(size, sizeof(size));
		match += boost::filesystem::path((*i)->filename()).filename().string();
		match += '\0';
		length += match.length();

		if (length > MulticastNetwork::BODY_MAXSIZE) {
			break;
		}

		answer += match;
	}

	m_network->send(
		*m_senderAddress,
		MulticastNetwork::Command::SearchFileNameReply,
		answer.c_str(), answer.length());
}

void MulticastHandler::on_searchFileNameReply(std::map<std::string, std::pair<float, std::string>> const& results)
{
	for (auto i = results.begin(); i != results.end(); ++i) {
		LOG_DEBUG("MatchReply: " << i->first);
	}
}

}
