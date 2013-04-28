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

	void connect(std::string const& ip, unsigned short port);
	void connect(boost::asio::ip::tcp::endpoint const& endpoint);

private:
	static unsigned short const UNICAST_PORT = 49370;

	void accept();
	void cleanupPeers();
	void on_accept(Peer::SharedPtr peer, boost::system::error_code const& ec);
	void on_connect(boost::system::error_code const& ec, Peer::SharedPtr const& peer);


	IOHandler& m_io;
	HashDatabase& m_db;

	boost::asio::ip::tcp::endpoint m_localEndpoint;
	boost::asio::ip::tcp::acceptor m_acceptor;
	std::deque<Peer::SharedPtr> m_peers;
};

}
