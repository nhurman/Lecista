#pragma once

#include <boost/asio.hpp>
#include "IOHandler.hpp"
#include "../Logger.hpp"

namespace Lecista {

class Peer
{
public:
	typedef boost::shared_ptr<Peer> SharedPtr;
	enum class Command : unsigned char
	{
		Block,

		NUM_COMMANDS
	};

	Peer(IOHandler& io);
	~Peer();

	boost::asio::ip::tcp::socket& socket() { return m_socket; }
	void initiate();

private:
	boost::asio::ip::tcp::socket m_socket;
	boost::asio::ip::tcp::endpoint m_remoteEndpoint;
	bool m_handshake;
	char m_readBuffer[500 * 1000]; // 500 kB

	void listen();
	void on_read(boost::system::error_code const& ec, size_t bytes);
};

}
