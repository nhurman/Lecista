#include "UnicastHandler.hpp"

namespace Lecista {

UnicastHandler::UnicastHandler(IOHandler& io, HashDatabase& db)
 : m_io(io), m_db(db),
   m_localEndpoint(boost::asio::ip::tcp::v4(), UNICAST_PORT),
   m_acceptor(io.ioService(), m_localEndpoint)
{
	LOG_DEBUG("Constructor");
	accept();

	m_io.start();
}

UnicastHandler::~UnicastHandler()
{
	LOG_DEBUG("Destructor");
	m_io.stop();
}

void UnicastHandler::connect(std::string const& ip, unsigned short port)
{
	connect(boost::asio::ip::tcp::endpoint(
		boost::asio::ip::address::from_string(ip), port));
}

void UnicastHandler::connect(boost::asio::ip::tcp::endpoint const& endpoint)
{
	Peer::SharedPtr peer(new Peer(m_io, m_db, boost::bind(&UnicastHandler::cleanupPeers, this)));
	peer->socket().async_connect(endpoint, boost::bind(&UnicastHandler::on_connect, this,
		boost::asio::placeholders::error, peer));
}

void UnicastHandler::accept()
{
	Peer::SharedPtr peer(new Peer(m_io, m_db, boost::bind(&UnicastHandler::cleanupPeers, this)));
	m_acceptor.async_accept(peer->socket(), boost::bind(
		&UnicastHandler::on_accept,
		this, peer, boost::asio::placeholders::error));
}

void UnicastHandler::cleanupPeers()
{
	std::deque<std::deque<Peer::SharedPtr>::iterator> toDelete;

	for (auto it = m_peers.begin(), e = m_peers.end(); it != e; ++it) {
		if ((*it)->state() == Peer::State::Closed) {
			toDelete.push_back(it);
		}
	}

	for (auto it: toDelete) {
		m_peers.erase(it);
	}
}

void UnicastHandler::on_accept(Peer::SharedPtr peer, boost::system::error_code const& ec)
{
	if (ec) {
		// Shutting down ?
		if (boost::asio::error::operation_aborted != ec) {
			LOG_DEBUG("Error in on_accept: " << ec.value());
		}

		return;
	}

	peer->init();
	m_peers.push_back(peer);
	accept();
}

void UnicastHandler::on_connect(boost::system::error_code const& ec, Peer::SharedPtr const& peer)
{
	if (ec) {
		LOG_DEBUG("Error in connect(): " << ec.value() << ": " << ec.message());
		return;
	}

	peer->download(m_db.find("/tmp/share/random")->file()->rootHash(), 0);
	m_peers.push_back(peer);
}

}
