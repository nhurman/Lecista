#include "GatewayElection.hpp"

namespace Lecista {

boost::posix_time::time_duration const GatewayElection::ELECTION_DURATION
	= boost::posix_time::milliseconds(5000);

GatewayElection::GatewayElection(MulticastNetwork* network) : m_network(network)
{
	m_inProgress = false;
	m_timer = m_network->ioHandler().createTimer();
	m_randomGenerator.seed(time(0) + getpid() * 10000);
	m_randomUInt = boost::random::uniform_int_distribution<uint32_t>(
		std::numeric_limits<uint32_t>::min(),
		std::numeric_limits<uint32_t>::max());
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

	apply();
}

void GatewayElection::registerCandidate(boost::asio::ip::address host, uint32_t id)
{
	if (m_inProgress) {
		if (m_candidates.find(id) == m_candidates.end()) {
			m_candidates[id] = host;
		}
		else { // Id collision
			LOG_DEBUG("Id collision!");
			if (m_candidates[id] != host) { // Not a duplicate packet
				// Restart election
				start();
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

		m_notifier(i->second, i->first == m_myId);

		if (i->first == m_myId) {
			LOG_DEBUG("I'm the new gateway!");
		}
		else {
			LOG_DEBUG("New gateway: " << i->second.to_string());
		}
	}
}

void GatewayElection::apply()
{
	m_myId = m_randomUInt(m_randomGenerator);
	uint32_t id = htonl(m_myId);
	m_network->send(MulticastNetwork::Command::Candidate, reinterpret_cast<char*>(&id), sizeof id);
	registerCandidate(boost::asio::ip::address_v4(0), m_myId);
}

}
