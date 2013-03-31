#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_array.hpp>
#include "../Logger.hpp"
#include "IOHandler.hpp"
#include "GatewayElection.hpp"

namespace Lecista {

class MulticastHandler
{
public:
	MulticastHandler(IOHandler& io);
	~MulticastHandler();

private:
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

	static boost::asio::ip::address const MCAST_ADDR;
	static unsigned short const MCAST_PORT;
	static unsigned char const HEADER_SIZE;

	IOHandler& m_io;
	boost::asio::ip::udp::socket* m_socket;
	boost::asio::ip::udp::endpoint m_senderEndpoint;
	boost::asio::ip::address m_senderAddress;
	char m_readBuffer[256];

	void on_read(boost::system::error_code ec, size_t bytes);
	void on_write(boost::system::error_code ec, size_t bytes, boost::shared_array<char> data);

	void listen();
	void send(boost::asio::ip::udp::endpoint dest, Command command, char const* data, char size);

	void on_candidate(uint32_t id);
	void on_discoverGateway();

	void on_electGateway();
	GatewayElection m_election;

	void on_hello(std::string name, float sharedSize);
	void on_message(std::string message);
	void on_searchBlock(std::string rootHash, uint32_t blockId);
	void on_searchFile(std::string filename);
};

}
