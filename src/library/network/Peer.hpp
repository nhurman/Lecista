#pragma once

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_array.hpp>
#include "IOHandler.hpp"
#include "../filesystem/HashDatabase.hpp"
#include "../filesystem/Hash.hpp"
#include "../Logger.hpp"

namespace Lecista {

class Peer : public boost::enable_shared_from_this<Peer>
{
public:
	typedef boost::shared_ptr<Peer> SharedPtr;
	enum class Command : unsigned char
	{
		DownloadBlock = 'D',
		UploadBlock = 'U',

		NUM_COMMANDS
	};


	enum class State
	{
		NewlyCreated,
		Idle,
		Uploading,
		Downloading,
		Closed
	};

	Peer(IOHandler& io, HashDatabase& db, boost::function<void()> handler);
	~Peer();

	boost::asio::ip::tcp::socket& socket() { return m_socket; }
	State state() { return m_state; }
	void init();

	void sendBlock(Hash::SharedPtr fileHash, unsigned int block);

private:

	// uint32_t  Body Size
	// char      Command
	// char[]    Body

	void listen();
	void on_read(boost::system::error_code const& ec, size_t bytes);
	void on_write(boost::shared_array<char> b, boost::system::error_code const& ec, uintmax_t start, uintmax_t end);
	void readBytes(size_t bytes = 0);
	void disconnect();

	boost::function<void()> m_onClose;
	IOHandler& m_io;
	HashDatabase& m_db;
	char m_readBuffer[500 * 1000]; // 500 kB
	boost::asio::ip::tcp::socket m_socket;
	HashDatabase::File::SharedPtr m_file;
	std::ifstream* m_fh;

	State m_state;
	unsigned int m_block;
	size_t m_uploaded;
	size_t m_downloaded;
	unsigned int m_uploadBandwidth;
	unsigned int m_downloadBandwidth;
};

}
