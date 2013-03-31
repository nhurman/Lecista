#include "MulticastNetwork.hpp"

using namespace boost::asio;

namespace Lecista {

ip::address const MulticastNetwork::MCAST_ADDR = ip::address::from_string("224.0.0.150");
unsigned short const MulticastNetwork::MCAST_PORT = 50400;
unsigned char const MulticastNetwork::HEADER_SIZE =
	sizeof(ip::address_v4::bytes_type) + sizeof(char);

MulticastNetwork::MulticastNetwork(IOHandler& io, Dispatcher dispatch)
: m_io(io), m_dispatch(dispatch)
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

MulticastNetwork::~MulticastNetwork()
{
	m_io.stop();
	delete m_socket;
}

void MulticastNetwork::listen()
{
	static boost::function<void(boost::system::error_code, size_t)> readHandler =
		boost::bind(
			&MulticastNetwork::on_read, this,
			placeholders::error,
			placeholders::bytes_transferred);
	static boost::asio::mutable_buffers_1 b =
		boost::asio::buffer(m_readBuffer, sizeof m_readBuffer);

	m_socket->async_receive_from(b, m_senderEndpoint, readHandler);
}

void MulticastNetwork::send(ip::udp::endpoint dest, Command command, char const* data, char size)
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
		&MulticastNetwork::on_write, this,
		placeholders::error,
		placeholders::bytes_transferred,
		packet);
	m_socket->async_send_to(boost::asio::buffer(packet.get(), HEADER_SIZE + 1 + size),
		dest, writeHandler);
}

void MulticastNetwork::on_read(boost::system::error_code ec, size_t bytes)
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
	boost::asio::ip::address senderAddress;
	if (0 == sourceIp) {
		senderAddress = m_senderEndpoint.address();
	}
	else {
		senderAddress = ip::address_v4(sourceIp);
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
	m_dispatch(senderAddress, command, args, argsSize);

	listen();
}

void MulticastNetwork::on_write(boost::system::error_code ec, size_t bytes, boost::shared_array<char> data)
{

}

}
