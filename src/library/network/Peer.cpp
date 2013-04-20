#include "Peer.hpp"

using namespace boost::asio;

namespace Lecista {

Peer::Peer(IOHandler& io) : m_socket(io.ioService()), m_handshake(false)
{
	LOG_DEBUG("Constructor");
}

Peer::~Peer()
{
	LOG_DEBUG("Destructor" << (m_handshake ? " " + m_socket.remote_endpoint().address().to_string() : ""));
}

void Peer::initiate()
{
	LOG_DEBUG("initiate() with " << m_socket.remote_endpoint().address().to_string());
	m_handshake = true;

	listen();
}

void Peer::listen()
{
	m_socket.async_read_some(
		boost::asio::buffer(m_readBuffer, sizeof m_readBuffer),
		boost::bind(&Peer::on_read, this,
			boost::asio::placeholders::error,
			placeholders::bytes_transferred));
}

void Peer::on_read(boost::system::error_code const& ec, size_t bytes)
{
	if (ec) {
		LOG_DEBUG("Error " << ec.value());
		return;
	}

	LOG_DEBUG("Got " << bytes << " bytes");
	listen();
}


}
