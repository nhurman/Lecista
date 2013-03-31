#include "IOHandler.hpp"

namespace Lecista {

IOHandler::IOHandler()
{
	boost::function<std::size_t()> run = boost::bind(&boost::asio::io_service::run, &m_ioService);
	m_thread = new boost::thread(run);
}

IOHandler::~IOHandler()
{
	delete m_thread;
}

}
