#include "MulticastHandler.hpp"

namespace Lecista {

MulticastHandler::MulticastHandler(IOHandler& io)
{
	m_network = new MulticastNetwork(io, boost::bind(&MulticastHandler::dispatch, this, _1, _2, _3, _4));
	m_gateway = new MulticastGateway(m_network);
	m_election = new GatewayElection(m_network);

	m_election->setNotifier(boost::bind(&MulticastGateway::update, m_gateway, _1, _2));
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
	char argsSize)
{
	typedef MulticastNetwork::Command Command;

	m_senderAddress = &senderAddress;

	if (Command::Candidate == command && sizeof(uint32_t) == argsSize) {
		on_candidate(ntohl(*reinterpret_cast<uint32_t*>(args)));
	}
	else if (Command::DiscoverGateway == command && 0 == argsSize) {
		on_discoverGateway();
	}
	else if (Command::ElectGateway == command && 0 == argsSize) {
		on_electGateway();
	}
	else if (Command::Hello == command && sizeof(float) < argsSize) {
		uint32_t b = ntohl(*reinterpret_cast<uint32_t*>(args + argsSize - sizeof(float)));
		on_hello(std::string(args, argsSize - sizeof(float)),
			*reinterpret_cast<float*>(&b));
	}
	else if (Command::Message == command && 0 < argsSize) {
		on_message(std::string(args, argsSize));
	}
	else if (Command::SearchBlock == command && sizeof(uint32_t) < argsSize) {
		on_searchBlock(std::string(args, argsSize - sizeof(uint32_t)),
			*reinterpret_cast<uint32_t*>(args + argsSize - sizeof(uint32_t)));
	}
	else if (Command::SearchFile == command && 0 < argsSize) {
		on_searchFile(std::string(args, argsSize));
	}
}

/* Message handling methods below */

void MulticastHandler::on_candidate(uint32_t id)
{
	LOG_DEBUG("on_candidate(" << id << ") from " << m_senderAddress->to_string());

	m_election->registerCandidate(*m_senderAddress, id);
}

void MulticastHandler::on_discoverGateway()
{
	LOG_DEBUG("on_discoverGateway() from " << m_senderAddress->to_string());
}

void MulticastHandler::on_electGateway()
{
	LOG_DEBUG("on_electGateway() from " << m_senderAddress->to_string());

	m_election->start();
}

void MulticastHandler::on_hello(std::string name, float sharedSize)
{
	LOG_DEBUG( "on_hello(" << name << ", " << sharedSize << ") from "	<< m_senderAddress->to_string());

	//ip::udp::endpoint ep(m_senderAddress, MCAST_PORT + 1);
	//send(ep, Command::DiscoverGateway, 0, 0);
}

void MulticastHandler::on_message(std::string message)
{
	LOG_DEBUG("on_message(" << message << ") from " << m_senderAddress->to_string());
}

void MulticastHandler::on_searchBlock(std::string rootHash, uint32_t blockId)
{
	LOG_DEBUG("on_searchBLock(" << rootHash << ", " << blockId << ") from " << m_senderAddress->to_string());
}

void MulticastHandler::on_searchFile(std::string filename)
{
	LOG_DEBUG("on_searchFile(" << filename << ") from " << m_senderAddress->to_string());
}

}
