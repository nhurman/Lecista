#include "Peer.hpp"

using namespace boost::asio;

namespace Lecista {

Peer::Peer(IOHandler& io, boost::function<void()> handler)
 : m_onClose(handler), m_io(io), m_socket(io.ioService()), m_state(State::NewlyCreated)
{
	LOG_DEBUG("Constructor");
	m_state = State::NewlyCreated;
}

Peer::~Peer()
{
	LOG_DEBUG("Destructor" << (m_state > State::NewlyCreated ? " " + m_socket.remote_endpoint().address().to_string() : ""));
	m_socket.close();
}

void Peer::initiate()
{
	LOG_DEBUG("initiate() with " << m_socket.remote_endpoint().address().to_string());

	listen();
	m_state = State::Idle;
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
		LOG_DEBUG("Error " << ec.value() << ": " << ec.message());
		m_state = State::Closed;
		close();
		return;
	}

	LOG_DEBUG("Got " << bytes << " bytes");
	listen();
}

void Peer::close()
{
	m_io.ioService().post(m_onClose);
}


}
