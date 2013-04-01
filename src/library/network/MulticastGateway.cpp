#include "MulticastGateway.hpp"

namespace Lecista {

MulticastGateway::MulticastGateway(MulticastNetwork* network)
{
	m_network = network;
	m_iAmTheGateway = false;
}

MulticastGateway::~MulticastGateway()
{

}

void MulticastGateway::update(boost::asio::ip::address address, bool isThisMe)
{
	m_gatewayAddress = address;
	m_iAmTheGateway = isThisMe;

	if (m_iAmTheGateway) {
		m_network->send(MulticastNetwork::Command::Gateway, 0, 0);
	}
}

void MulticastGateway::on_discoverGateway(boost::asio::ip::address senderAddress)
{
	if (m_iAmTheGateway) {
		LOG_DEBUG("Answering DiscoverGateway from " << senderAddress.to_string());
		m_network->send(senderAddress, MulticastNetwork::Command::Gateway, 0, 0);
	}
}

}
