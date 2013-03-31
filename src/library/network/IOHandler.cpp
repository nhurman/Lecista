#include "IOHandler.hpp"

namespace Lecista {

IOHandler::IOHandler()
{
	m_thread = 0;
}

IOHandler::~IOHandler()
{
	stop();
}

boost::asio::ip::udp::socket* IOHandler::createUdpSocket()
{
	return new boost::asio::ip::udp::socket(m_ioService);
}

boost::asio::deadline_timer* IOHandler::createTimer()
{
	return new boost::asio::deadline_timer(m_ioService);
}

void IOHandler::start()
{
	if (0 == m_thread) {
		LOG_DEBUG("Starting io processing thread");
		boost::function<std::size_t()> run = boost::bind(&boost::asio::io_service::run, &m_ioService);
		m_thread = new boost::thread(run);
	}
}

void IOHandler::stop()
{
	if (0 != m_thread) {
		LOG_DEBUG("Stopping io processing thread");
		if (!m_thread->try_join_for(boost::chrono::milliseconds(500))) {
			m_thread->interrupt();
		}
		delete m_thread;
		m_thread = 0;
	}
}

}
