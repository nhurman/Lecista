#pragma once

#include <deque>
#include <boost/asio.hpp>
#include "IOHandler.hpp"
#include "Peer.hpp"
#include "../filesystem/HashDatabase.hpp"
#include "../Logger.hpp"

namespace Lecista {

class UnicastHandler
{
public:
	UnicastHandler(IOHandler& io, HashDatabase& db);
	~UnicastHandler();

private:
	// uint32_t  Body Size
	// char      Command
	// char[]    Body

	enum {
		HEADER_SIZE = sizeof(uint32_t) + sizeof(char),
		UNICAST_PORT = 49370,
	};

	void accept();
	void on_accept(Peer::SharedPtr peer, boost::system::error_code const& ec);

	IOHandler& m_io;
	HashDatabase& m_db;

	boost::asio::ip::tcp::endpoint m_localEndpoint;
	boost::asio::ip::tcp::acceptor m_acceptor;

	std::deque<Peer::SharedPtr> m_peers;
};

}
