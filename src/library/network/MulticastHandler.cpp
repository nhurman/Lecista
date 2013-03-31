#include "MulticastHandler.hpp"

using namespace boost::asio;

namespace Lecista {

char const* MulticastHandler::MCAST_ADDR = "224.0.0.150";
unsigned short const MulticastHandler::MCAST_PORT = 50400;
unsigned char const MulticastHandler::HEADER_SIZE =
	sizeof(ip::address_v4::bytes_type) + sizeof(char);

MulticastHandler::MulticastHandler(IOHandler& io) : m_io(io), m_socket(io.ioService())
{
	// Set up multicast server
	ip::address listenAddress = ip::address::from_string("0.0.0.0");
	ip::udp::endpoint listenEndpoint(listenAddress, MCAST_PORT);
	m_socket.open(listenEndpoint.protocol());
	//m_socket.set_option(ip::udp::socket::reuse_address(true));
	m_socket.bind(listenEndpoint);

	// Join the multicast group
	ip::address multicastAddress = ip::address::from_string(MCAST_ADDR);
	m_socket.set_option(ip::multicast::join_group(multicastAddress));

	// Listen for messages
	listen();
}

void MulticastHandler::listen()
{
	static boost::function<void(boost::system::error_code, size_t)> readHandler =
		boost::bind(
			&MulticastHandler::on_newData, this,
			placeholders::error,
			placeholders::bytes_transferred);
	m_socket.async_receive_from(
		boost::asio::buffer(m_buffer, sizeof m_buffer),
		m_senderEndpoint, readHandler);
}

void MulticastHandler::send(ip::udp::endpoint dest, Command command, char* data, char size)
{
	char packet[HEADER_SIZE + 1 + size];

	// Build header, 0.0.0.0 ip means "Use the one you see the packet coming from"
	std::memset(packet, 0, sizeof(ip::address_v4::bytes_type));
	packet[HEADER_SIZE - 1] = size + 1;

	// Append command and arguments
	packet[HEADER_SIZE] = static_cast<char>(command);
	std::memcpy(&packet[HEADER_SIZE + 1], data, size);

	// Send
	m_socket.async_send_to(
		boost::asio::buffer(packet, sizeof packet),
		dest, &nullHandler);

	std::cout << "I just sent a packet to " << dest.address().to_string()
		<< ":" << dest.port() << std::endl;
	std::cout.write(packet, sizeof packet);
	std::cout << std::endl;
}

void MulticastHandler::on_newData(boost::system::error_code ec, size_t bytes)
{
	if (bytes < HEADER_SIZE) { // Incomplete packet, drop it
		listen();
		return;
	}

	// Parse header
	ip::address_v4::bytes_type sourceIp;
	std::memcpy(&sourceIp, m_buffer, sizeof sourceIp);
	ip::address_v4 sourceAddress_v4 = ip::address_v4(sourceIp);
	m_senderAddress = sourceAddress_v4;

	static ip::address nullIp = ip::address::from_string("0.0.0.0");
	if (m_senderAddress == nullIp) {
		m_senderAddress = m_senderEndpoint.address();
	}

	unsigned char bodyLength = *(char*)(m_buffer + sizeof sourceIp);

	// Incomplete packet, drop it
	if (bodyLength < sizeof(Command) || bytes != (HEADER_SIZE + bodyLength)) {
		listen();
		return;
	}

	// Extract command id
	Command command;
	std::memcpy(&command, m_buffer + HEADER_SIZE, sizeof command);

	if (command >= Command::NUM_COMMANDS) {
		listen();
		return;
	}

	char* args = m_buffer + HEADER_SIZE + 1;
	char argsSize = bodyLength - 1;

	// Dispatch command to handler
	if (Command::Candidate == command && sizeof(uint32_t) == argsSize) {
		on_candidate(*(uint32_t*)args);
	}
	else if (Command::DiscoverGateway == command && 0 == argsSize) {
		on_discoverGateway();
	}
	else if (Command::ElectGateway == command && 0 == argsSize) {
		on_electGateway();
	}
	else if (Command::Hello == command && sizeof(float) < argsSize) {
		on_hello(std::string(args, argsSize - sizeof(float)),
			*(float*)(args + argsSize - sizeof(float)));
	}
	else if (Command::Message == command && 0 < argsSize) {
		on_message(std::string(args, argsSize));
	}
	else if (Command::SearchBlock == command && sizeof(uint32_t) < argsSize) {
		on_searchBlock(std::string(args, argsSize - sizeof(uint32_t)),
			*(uint32_t*)(args + argsSize - sizeof(uint32_t)));
	}
	else if (Command::SearchFile == command && 0 < argsSize) {
		on_searchFile(std::string(args, argsSize));
	}

	listen();
}

/* Message handling methods below */

void MulticastHandler::on_candidate(uint32_t id)
{
	std::cout << "on_candidate(" << id << ") from "
		<< m_senderAddress.to_string() << std::endl;
}

void MulticastHandler::on_discoverGateway() {}
void MulticastHandler::on_electGateway() {}
void MulticastHandler::on_hello(std::string name, float sharedSize)
{
	std::cout << "on_hello(" << name << ", " << sharedSize << ")" << std::endl;
}

void MulticastHandler::on_message(std::string message) {}
void MulticastHandler::on_searchBlock(std::string rootHash, uint32_t blockId) {}
void MulticastHandler::on_searchFile(std::string filename) {}

}
