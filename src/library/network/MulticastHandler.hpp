#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_array.hpp>
#include "../Logger.hpp"
#include "IOHandler.hpp"
#include "MulticastNetwork.hpp"
#include "MulticastGateway.hpp"
#include "GatewayElection.hpp"

namespace Lecista {

class MulticastHandler
{
public:
	MulticastHandler(IOHandler& io);
	~MulticastHandler();

private:
	MulticastNetwork* m_network;
	boost::asio::ip::address* m_senderAddress;

	void dispatch(
		boost::asio::ip::address senderAddress,
		MulticastNetwork::Command command,
		char *args,
		char argsSize,
		bool forward);

	MulticastGateway* m_gateway;
	GatewayElection* m_election;

	void on_candidate(uint32_t id);
	void on_electGateway();
	void on_forward(MulticastNetwork::Command command, char* args, char argsSize);
	void on_gateway();
	void on_hello(std::string name, float sharedSize);
	void on_message(std::string message);
	void on_remoteGateway();
	void on_searchBlock(std::string rootHash, uint32_t blockId);
	void on_searchFile(std::string filename);
};

}
