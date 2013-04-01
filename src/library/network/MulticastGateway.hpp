#pragma once

#include <vector>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "MulticastNetwork.hpp"
#include "../Logger.hpp"

namespace Lecista {

class MulticastGateway
{
public:
	MulticastGateway(MulticastNetwork* network);
	~MulticastGateway();

	void update(boost::asio::ip::address address, bool isThisMe);
	void initializeGateway();

	void on_discoverGateway(boost::asio::ip::address const& sender);
	void on_remoteGateway(boost::asio::ip::address const& sender, uint32_t id);

private:
	struct Network
	{
		uint32_t ip;
		char cidr;

		boost::asio::ip::address network;
		boost::asio::ip::address broadcast;

		bool operator<(Network const& other) const { return network < other.network; }
		bool operator>(Network const& other) const { return network > other.network; }
	};

	MulticastNetwork* m_network;
	std::vector<Network> m_networks;
	std::map<Network, boost::asio::ip::address> m_gateways;

	boost::asio::ip::address m_gatewayAddress;
	bool m_iAmTheGateway;
	uint32_t m_myId;
};

}
