#pragma once

#include <map>
#include <ctime>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "MulticastNetwork.hpp"
#include "../Logger.hpp"

namespace Lecista {

class GatewayElection
{
public:
	//! Function that gets called with the result of the election.
	typedef boost::function<void(boost::asio::ip::address newGateway, bool isThisMe)> Notifier;

	//!
	/*!
	  \param network
	*/
	GatewayElection(MulticastNetwork* network, Notifier notifier);
	~GatewayElection();

	//! Is there an election in progress ?
	/*!
	  \return True if an election is in progress
	*/
	bool inProgress() { return m_inProgress; }
	void gatewayTimeout();
	void start();

	//!
	void registerCandidate(boost::asio::ip::address host, uint32_t id);

private:
	static boost::posix_time::time_duration const ELECTION_DURATION;

	MulticastNetwork* m_network;
	bool m_inProgress;
	uint32_t m_myId;
	Notifier m_notifier;
	boost::asio::deadline_timer* m_timer;
	std::map<uint32_t, boost::asio::ip::address> m_candidates;

	void timeOut(boost::system::error_code ec);
	void apply();
};

}
