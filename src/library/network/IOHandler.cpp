#include "IOHandler.hpp"

namespace Lecista {

IOHandler::IOHandler()
{
	m_thread = 0;
}

IOHandler::~IOHandler()
{
	if (0 != m_thread) {
		delete m_thread;
	}
}

boost::asio::ip::udp::socket* IOHandler::createUdpSocket()
{
	 return new boost::asio::ip::udp::socket(m_ioService);
}

void IOHandler::createThread()
{
	if (0 == m_thread) {
		boost::function<std::size_t()> run = boost::bind(&boost::asio::io_service::run, &m_ioService);
		m_thread = new boost::thread(run);
	}
}

}
