#include "MulticastHandler.hpp"

using namespace boost::asio;

namespace Lecista {

ip::address const MulticastHandler::MCAST_ADDR = ip::address::from_string("224.0.0.150");
unsigned short const MulticastHandler::MCAST_PORT = 50400;
unsigned char const MulticastHandler::HEADER_SIZE =
	sizeof(ip::address_v4::bytes_type) + sizeof(char);

MulticastHandler::MulticastHandler(IOHandler& io)
: m_io(io), m_election(io)
{
	m_socket = m_io.createUdpSocket();

	// Set up multicast server
	ip::address listenAddress = ip::address_v4(0);
	ip::udp::endpoint listenEndpoint(listenAddress, MCAST_PORT);
	m_socket->open(listenEndpoint.protocol());
	m_socket->set_option(ip::udp::socket::reuse_address(true));
	m_socket->bind(listenEndpoint);

	// Join the multicast group
	m_socket->set_option(ip::multicast::join_group(MCAST_ADDR));

	// Listen for messages
	listen();
	m_io.start();
}

MulticastHandler::~MulticastHandler()
{
	m_io.stop();
	delete m_socket;
}

void MulticastHandler::listen()
{
	static boost::function<void(boost::system::error_code, size_t)> readHandler =
		boost::bind(
			&MulticastHandler::on_read, this,
			placeholders::error,
			placeholders::bytes_transferred);
	static boost::asio::mutable_buffers_1 b =
		boost::asio::buffer(m_readBuffer, sizeof m_readBuffer);

	m_socket->async_receive_from(b, m_senderEndpoint, readHandler);
}

void MulticastHandler::send(ip::udp::endpoint dest, Command command, char const* data, char size)
{
	boost::shared_array<char> packet(new char[HEADER_SIZE + 1 + size]);

	// Build header, 0.0.0.0 ip means "Use the one you see the packet coming from
	packet[0] = packet[1] = packet[2] = packet[3] = 0;
	packet[HEADER_SIZE - 1] = size + 1;

	// Append command and arguments
	packet[HEADER_SIZE] = static_cast<char>(command);
	std::memcpy(&packet[HEADER_SIZE + 1], data, size);

	// Send
	static auto writeHandler = boost::bind(
		&MulticastHandler::on_write, this,
		placeholders::error,
		placeholders::bytes_transferred,
		packet);
	m_socket->async_send_to(boost::asio::buffer(packet.get(), HEADER_SIZE + 1 + size),
		dest, writeHandler);
}

void MulticastHandler::on_read(boost::system::error_code ec, size_t bytes)
{
	if (ec) {
		// Shutting down ?
		if (boost::asio::error::operation_aborted != ec) {
			LOG_DEBUG("Error in on_read: " << ec.value());
		}

		return;
	}


	if (bytes < HEADER_SIZE + 1) { // Incomplete packet, drop it
		listen();
		return;
	}

	// Parse header
	uint32_t sourceIp = ntohl(*reinterpret_cast<uint32_t*>(m_readBuffer));
	if (0 == sourceIp) {
		m_senderAddress = m_senderEndpoint.address();
	}
	else {
		m_senderAddress = ip::address_v4(sourceIp);
	}

	unsigned char bodyLength = m_readBuffer[sizeof sourceIp];

	// Incomplete packet, drop it
	if (bodyLength < sizeof(Command) || bytes != (HEADER_SIZE + bodyLength)) {
		listen();
		return;
	}

	// Extract command id
	Command command = static_cast<Command>(m_readBuffer[HEADER_SIZE]);
	if (command >= Command::NUM_COMMANDS) {
		listen();
		return;
	}

	char* args = m_readBuffer + HEADER_SIZE + 1;
	char argsSize = bodyLength - 1;

	// Dispatch command to handler
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

	listen();
}

void MulticastHandler::on_write(boost::system::error_code ec, size_t bytes, boost::shared_array<char> data)
{

}

/* Message handling methods below */

void MulticastHandler::on_candidate(uint32_t id)
{
	LOG_DEBUG("on_candidate(" << id << ") from " << m_senderAddress.to_string());

	m_election.registerCandidate(m_senderAddress, id);
}

void MulticastHandler::on_discoverGateway()
{
	LOG_DEBUG("on_discoverGateway() from " << m_senderAddress.to_string());
}

void MulticastHandler::on_electGateway()
{
	LOG_DEBUG("on_electGateway() from " << m_senderAddress.to_string());

	m_election.start();
}

void MulticastHandler::on_hello(std::string name, float sharedSize)
{
	LOG_DEBUG( "on_hello(" << name << ", " << sharedSize << ") from "	<< m_senderAddress.to_string());

	ip::udp::endpoint ep(m_senderAddress, MCAST_PORT + 1);
	send(ep, Command::DiscoverGateway, 0, 0);
}

void MulticastHandler::on_message(std::string message)
{
	LOG_DEBUG("on_message(" << message << ") from " << m_senderAddress.to_string());
}

void MulticastHandler::on_searchBlock(std::string rootHash, uint32_t blockId)
{
	LOG_DEBUG("on_searchBLock(" << rootHash << ", " << blockId << ") from " << m_senderAddress.to_string());
}

void MulticastHandler::on_searchFile(std::string filename)
{
	LOG_DEBUG("on_searchFile(" << filename << ") from " << m_senderAddress.to_string());
}

}
