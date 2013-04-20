#pragma once

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/thread/thread.hpp>
#include <boost/random/taus88.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "../Logger.hpp"

namespace Lecista {

class IOHandler
{
public:
	IOHandler();
	~IOHandler();

	boost::asio::io_service& ioService() { return m_ioService; }
	boost::asio::ip::udp::socket* createUdpSocket();
	boost::asio::deadline_timer* createTimer();
	void start();
	void stop();
	uint32_t randomUInt();

private:
	boost::asio::io_service m_ioService;
	boost::thread* m_thread;
	boost::taus88 m_randomGenerator;
	boost::random::uniform_int_distribution<uint32_t> m_randomUInt;
};

}
