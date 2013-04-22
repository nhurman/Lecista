#pragma once

#include <boost/asio.hpp>
#include "IOHandler.hpp"
#include "../filesystem/Hash.hpp"
#include "../Logger.hpp"

namespace Lecista {

class Peer
{
public:
	typedef boost::shared_ptr<Peer> SharedPtr;
	enum class Command : unsigned char
	{
		Block,

		NUM_COMMANDS
	};


	enum class State
	{
		NewlyCreated,
		Idle,
		SendingBlock,
		ReceivingBlock,
		Closed
	};

	Peer(IOHandler& io, boost::function<void()> handler);
	~Peer();

	boost::asio::ip::tcp::socket& socket() { return m_socket; }
	State state() { return m_state; }
	void initiate();

private:
	void listen();
	void on_read(boost::system::error_code const& ec, size_t bytes);
	void close();

	boost::function<void()> m_onClose;
	IOHandler& m_io;
	char m_readBuffer[500 * 1000]; // 500 kB
	boost::asio::ip::tcp::socket m_socket;
	boost::asio::ip::tcp::endpoint m_remoteEndpoint;
	Hash::SharedPtr m_hash;

	State m_state;
	unsigned int m_block;
	size_t m_uploaded;
	size_t m_downloaded;
	unsigned int m_uploadBandwidth;
	unsigned int m_downloadBandwidth;
};

}
