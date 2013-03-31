#pragma once

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/thread/thread.hpp>
#include "../Logger.hpp"

namespace Lecista {

class IOHandler
{
public:
	IOHandler();
	~IOHandler();

	boost::asio::ip::udp::socket* createUdpSocket();
	boost::asio::deadline_timer* createTimer();
	void start();
	void stop();

private:
	boost::asio::io_service m_ioService;
	boost::thread* m_thread;
};

}
