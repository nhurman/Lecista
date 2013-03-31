#include "GatewayElection.hpp"

namespace Lecista {

boost::posix_time::time_duration const GatewayElection::ELECTION_DURATION
= boost::posix_time::milliseconds(5000);

GatewayElection::GatewayElection(IOHandler& io) : m_io(io), m_inProgress(false)
{
	m_timer = m_io.createTimer();
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
				if (m_candidates[id] > host) { // Keep the smallest ip address
					m_candidates[id] = host;
				}
			}
		}
	}
}

void GatewayElection::timeOut(boost::system::error_code ec)
{
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

		LOG_DEBUG("New gateway elected: " << i->second.to_string());
	}
}

}
