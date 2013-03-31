#pragma once

#include <boost/asio.hpp>
#include "MulticastNetwork.hpp"
#include "../Logger.hpp"

namespace Lecista {

class MulticastGateway
{
public:
	MulticastGateway(MulticastNetwork* network);
	~MulticastGateway();

	void update(boost::asio::ip::address address, bool isThisMe);

private:
	MulticastNetwork* m_network;

	boost::asio::ip::address m_gatewayAddress;
	bool m_iAmTheGateway;
};

}
