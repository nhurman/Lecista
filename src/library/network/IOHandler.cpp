#include "IOHandler.hpp"

namespace Lecista {

IOHandler::IOHandler()
{
	m_thread = 0;

	m_randomGenerator.seed(static_cast<unsigned int>(time(0)) + _getpid() * 10000);
	m_randomUInt = boost::random::uniform_int_distribution<uint32_t>(
		std::numeric_limits<uint32_t>::min(),
		std::numeric_limits<uint32_t>::max());
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
		m_ioService.stop();
		if (!m_thread->try_join_for(boost::chrono::milliseconds(500))) {
			m_thread->interrupt();
		}

		delete m_thread;
		m_thread = 0;
	}
}

uint32_t IOHandler::randomUInt()
{
	return m_randomUInt(m_randomGenerator);
}

}
