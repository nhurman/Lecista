#include "MulticastGateway.hpp"

namespace Lecista {

boost::posix_time::time_duration const MulticastGateway::PING_INTERVAL
	= boost::posix_time::milliseconds(2000);

MulticastGateway::MulticastGateway(MulticastNetwork* network) : m_networks(5)
{
	m_network = network;
	m_iAmTheGateway = false;

	m_networks[0].ip = 0x0a850800; m_networks[0].cidr = 24; // 10.133.8.0  Asso
	m_networks[1].ip = 0x0a851000; m_networks[1].cidr = 21; // 10.133.16.0 Glénan
	m_networks[2].ip = 0x0a851800; m_networks[2].cidr = 23; // 10.133.24.0 Arz
	m_networks[3].ip = 0x0a851a00; m_networks[3].cidr = 23; // 10.133.26.0 Bréhat
	m_networks[4].ip = 0x0a851c00; m_networks[4].cidr = 23; // 10.133.28.0 Cézembre

	// Calculate network and broadcast addresses
	for (int i = 0; i < m_networks.size(); i++) {
		uint32_t network = m_networks[i].ip;
		network &= ~0 << (32 - m_networks[i].cidr);
		m_networks[i].network = boost::asio::ip::address_v4(network);

		uint32_t broadcast = network;
		broadcast += 1 << (32 - m_networks[i].cidr);
		broadcast -= 1;
		m_networks[i].broadcast = boost::asio::ip::address_v4(broadcast);
	}

	// Commands that need to be forwarded
	m_forwardCommands.insert(MulticastNetwork::Command::Hello);
	m_forwardCommands.insert(MulticastNetwork::Command::Message);
	m_forwardCommands.insert(MulticastNetwork::Command::SearchBlock);
	m_forwardCommands.insert(MulticastNetwork::Command::SearchFileName);

	// Alive ping
	m_onTimeout = 0;
	m_timer = m_network->ioHandler().createTimer();
	pingTimer(boost::system::error_code());
}

MulticastGateway::~MulticastGateway()
{
	m_timer->cancel();
	delete m_timer;
}

void MulticastGateway::onTimeout(boost::function<void()> callback)
{
	m_onTimeout = callback;
}

void MulticastGateway::update(boost::asio::ip::address address, bool isThisMe)
{
	m_gatewayAddress = address;
	m_iAmTheGateway = isThisMe;

	if (m_iAmTheGateway) {
		initializeGateway();
	}
}

void MulticastGateway::initializeGateway()
{
	if (!m_iAmTheGateway) {
		return;
	}

	for (auto i = m_networks.begin(); i != m_networks.end(); ++i) {
		m_network->send(i->broadcast, MulticastNetwork::Command::RemoteGateway);
	}
}

void MulticastGateway::pingTimer(boost::system::error_code ec)
{
	if (ec) {
		return;
	}

	if (m_iAmTheGateway) {
		m_network->send(MulticastNetwork::Command::Gateway);
	}
	else {
		m_missedPings++;

		if (m_missedPings > 2 && 0 != m_onTimeout) {
			m_onTimeout();
		}
	}

	m_timer->expires_from_now(PING_INTERVAL);
	m_timer->async_wait(boost::bind(
		&MulticastGateway::pingTimer, this,
		boost::asio::placeholders::error));
}

void MulticastGateway::on_forward(
	boost::asio::ip::address sender,
	MulticastNetwork::Command command,
	char* data,
	char size)
{
	if (!m_iAmTheGateway) {
		return;
	}

	// Check this command is forwardable
	if (m_forwardCommands.find(command) == m_forwardCommands.end()) {
		return;
	}

	// Send it on our network
	m_network->send(command, data, size, &sender);
	m_network->injectForwarded(sender, command, data, size);
}

void MulticastGateway::on_gateway(boost::asio::ip::address const& sender)
{
	m_missedPings = 0;
}

void MulticastGateway::on_remoteGateway(boost::asio::ip::address const& sender)
{
	if (!m_iAmTheGateway) {
		return;
	}

	for (auto i = m_networks.begin(); i != m_networks.end(); ++i) {
		if (i->network < sender && sender < i->broadcast) {
			LOG_DEBUG("Setting gateway for " << i->network.to_string()
				<< "/" << (int)i->cidr << " to " << sender.to_string());
			m_gateways[*i] = sender;
			break;
		}
	}
}

void MulticastGateway::forward(
	boost::asio::ip::address sender,
	MulticastNetwork::Command command,
	char* args, char argsSize)
{
	if (!m_iAmTheGateway ||
		m_forwardCommands.find(command) == m_forwardCommands.end()) {
		return;
	}

	char *forwardData = new char[argsSize + 1];
	std::memcpy(forwardData + 1, args, argsSize);
	forwardData[0] = static_cast<int>(command);

	for (auto i = m_gateways.begin(); i != m_gateways.end(); ++i) {
		LOG_DEBUG("Forwarding to " << i->second.to_string());
		m_network->send(
			i->second,
			MulticastNetwork::Command::Forward,
			forwardData,
			argsSize + 1,
			&sender);
	}

	delete[] forwardData;
}

}
