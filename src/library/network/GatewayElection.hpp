#pragma once

#include <map>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "IOHandler.hpp"
#include "../Logger.hpp"

namespace Lecista {

class GatewayElection
{
public:
	typedef boost::function<void(boost::asio::ip::address newGateway)> Notifier;

	GatewayElection(IOHandler& io);
	~GatewayElection();

	bool inProgress() { return m_inProgress; }
	void start();
	void registerCandidate(boost::asio::ip::address host, uint32_t id);
	void setNotifier(Notifier notifier);

private:
	static boost::posix_time::time_duration const ELECTION_DURATION;

	IOHandler& m_io;
	bool m_inProgress;
	Notifier m_notifier;
	boost::asio::deadline_timer* m_timer;
	std::map<uint32_t, boost::asio::ip::address> m_candidates;

	void timeOut(boost::system::error_code ec);
};

}
