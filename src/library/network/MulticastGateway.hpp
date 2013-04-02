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

	void onTimeout(boost::function<void()> callback);

	void update(boost::asio::ip::address address, bool isThisMe);
	void forward(
		boost::asio::ip::address sender,
		MulticastNetwork::Command command,
		char* args, char argsSize);

	void on_gateway(boost::asio::ip::address const& sender);
	void on_forward(
		boost::asio::ip::address sender,
		MulticastNetwork::Command command,
		char* data,
		char size);
	void on_remoteGateway(boost::asio::ip::address const& sender);

private:
	static boost::posix_time::time_duration const PING_INTERVAL;

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
	std::set<MulticastNetwork::Command> m_forwardCommands;
	boost::function<void()> m_onTimeout;

	boost::asio::ip::address m_gatewayAddress;
	bool m_iAmTheGateway;
	boost::asio::deadline_timer* m_timer;
	int m_missedPings;

	void initializeGateway();
	void pingTimer(boost::system::error_code ec);
};

}
