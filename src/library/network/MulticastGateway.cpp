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
	LOG_DEBUG("update(" << address.to_string() << ", " << isThisMe << ")");

	m_gatewayAddress = address;
	m_iAmTheGateway = isThisMe;
}

}
