#pragma once

#include <map>
#include <ctime>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/random/taus88.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "MulticastNetwork.hpp"
#include "../Logger.hpp"

namespace Lecista {

class MulticastHandler;

class GatewayElection
{
public:
	typedef boost::function<void(boost::asio::ip::address newGateway, bool isThisMe)> Notifier;

	GatewayElection(MulticastNetwork* network);
	~GatewayElection();

	bool inProgress() { return m_inProgress; }
	void start();
	void registerCandidate(boost::asio::ip::address host, uint32_t id);
	void setNotifier(Notifier notifier);

private:
	static boost::posix_time::time_duration const ELECTION_DURATION;

	MulticastNetwork* m_network;
	boost::taus88 m_randomGenerator;
	boost::random::uniform_int_distribution<uint32_t> m_randomUInt;
	bool m_inProgress;
	uint32_t m_myId;
	Notifier m_notifier;
	boost::asio::deadline_timer* m_timer;
	std::map<uint32_t, boost::asio::ip::address> m_candidates;

	void timeOut(boost::system::error_code ec);
	void apply();
};

}
