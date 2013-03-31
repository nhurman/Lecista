#pragma once

#include <cstring>
#include "IOHandler.hpp"

namespace Lecista {

class MulticastHandler
{
public:
	static char const* MCAST_ADDR;
	static unsigned short const MCAST_PORT;
	static unsigned char const HEADER_SIZE;

	enum class Command : unsigned char
	{
		Candidate,
		DiscoverGateway,
		ElectGateway,
		Hello,
		Message,
		SearchBlock,
		SearchFile,

		NUM_COMMANDS
	};

	MulticastHandler(IOHandler& io);
	void listen();
	void send(boost::asio::ip::udp::endpoint dest, Command command, char* data, char size);
	void on_newData(boost::system::error_code ec, size_t bytes);

	void on_candidate(uint32_t id);
	void on_discoverGateway();
	void on_electGateway();
	void on_hello(std::string name, float sharedSize);
	void on_message(std::string message);
	void on_searchBlock(std::string rootHash, uint32_t blockId);
	void on_searchFile(std::string filename);

private:
	IOHandler& m_io;
	boost::asio::ip::udp::socket m_socket;
	boost::asio::ip::udp::endpoint m_senderEndpoint;
	boost::asio::ip::address m_senderAddress;
	char m_buffer[256];
	char m_buffer2[256];

	static void nullHandler(boost::system::error_code, size_t) {}
};

}
