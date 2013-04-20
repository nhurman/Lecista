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

void UnicastHandler::accept()
{
	Peer::SharedPtr peer(new Peer(m_io));
	m_acceptor.async_accept(peer->socket(), boost::bind(
		&UnicastHandler::on_accept,
		this, peer, boost::asio::placeholders::error));
}

void UnicastHandler::on_accept(Peer::SharedPtr peer, boost::system::error_code const& ec)
{
	if (ec) {
		// Shutting down ?
		if (boost::asio::error::operation_aborted != ec) {
			LOG_DEBUG("Error in on_read: " << ec.value());
		}

		return;
	}

	peer->initiate();
	m_peers.push_back(peer);

	accept();
}

}
