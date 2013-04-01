#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_array.hpp>
#include "IOHandler.hpp"

namespace Lecista {

class MulticastNetwork
{
public:
	enum class Command : unsigned char
	{
		Candidate,
		DiscoverGateway,
		ElectGateway,
		Forward,
		Gateway,
		Hello,
		Message,
		RemoteGateway,
		SearchBlock,
		SearchFile,

		NUM_COMMANDS
	};

	typedef boost::function<void(
		boost::asio::ip::address m_senderAddress,
		MulticastNetwork::Command command,
		char *args,
		char argsSize)> Dispatcher;

	MulticastNetwork(IOHandler& io, Dispatcher dispatch);
	~MulticastNetwork();

	IOHandler& ioHandler() { return m_io; }

	void listen();
	void send(
		Command command,
		char const* data = 0,
		char size = 0,
		boost::asio::ip::address* sender = 0);
	void send(
		boost::asio::ip::address dest,
		Command command,
		char const* data = 0,
		char size = 0,
		boost::asio::ip::address* sender = 0);
	void send(
		boost::asio::ip::udp::endpoint dest,
		Command command,
		char const* data = 0,
		char size = 0,
		boost::asio::ip::address* sender = 0);

private:
	static boost::asio::ip::address const MCAST_ADDR;
	static unsigned short const MCAST_PORT;
	static unsigned char const HEADER_SIZE;

	IOHandler& m_io;
	boost::asio::ip::udp::socket* m_socket;
	boost::asio::ip::udp::endpoint m_senderEndpoint;
	boost::asio::ip::address m_senderAddress;
	char m_readBuffer[256];

	Dispatcher m_dispatch;
	void on_read(boost::system::error_code ec, size_t bytes);
	void on_write(boost::system::error_code ec, size_t bytes, boost::shared_array<char> data);
};

}
