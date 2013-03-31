#include "GatewayElection.hpp"

namespace Lecista {

boost::posix_time::time_duration const GatewayElection::ELECTION_DURATION
	= boost::posix_time::milliseconds(5000);

GatewayElection::GatewayElection(MulticastNetwork* network) : m_network(network)
{
	m_inProgress = false;
	m_timer = m_network->ioHandler().createTimer();
}

GatewayElection::~GatewayElection()
{
	delete m_timer;
}

void GatewayElection::start()
{
	if (m_inProgress) { // Stop current election
		m_timer->cancel();
	}

	m_candidates.clear();
	m_timer->expires_from_now(ELECTION_DURATION);
	m_timer->async_wait(boost::bind(
		&GatewayElection::timeOut, this,
		boost::asio::placeholders::error));
	m_inProgress = true;
}

void GatewayElection::registerCandidate(boost::asio::ip::address host, uint32_t id)
{
	if (m_inProgress) {
		if (m_candidates.find(id) == m_candidates.end()) {
			m_candidates[id] = host;
		}
		else { // Id collision
			if (m_candidates[id] != host) { // Not a duplicate packet
				// Restart election
				start();
				std::terminate();
			}
		}
	}
}

void GatewayElection::setNotifier(Notifier notifier)
{
	m_notifier = notifier;
}

void GatewayElection::timeOut(boost::system::error_code ec)
{
	m_inProgress = false;

	if (ec) {
		if (boost::asio::error::operation_aborted != ec) {
			LOG_DEBUG("Error in gateway election: " << ec.value());
		}
		return;
	}

	LOG_DEBUG("Gateway election finished");

	int elected = 0;
	int numCandidates = m_candidates.size();

	if (numCandidates > 0) {
		for (auto i = m_candidates.begin(); i != m_candidates.end(); ++i) {
			elected = (elected + i->first) % numCandidates;
		}

		auto i = m_candidates.begin();
		while (elected > 0) {
			--elected;
			++i;
		}

		m_gateway = i->second;
		LOG_DEBUG("New gateway: " << i->second.to_string());
	}
}

}
