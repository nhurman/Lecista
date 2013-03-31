#pragma once

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

namespace Lecista {

class IOHandler
{
public:
	IOHandler();
	~IOHandler();

	boost::asio::io_service& ioService() { return m_ioService; }

private:
	boost::asio::io_service m_ioService;
	boost::thread* m_thread;
};

}
