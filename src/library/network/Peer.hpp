#pragma once

#include <vector>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_array.hpp>
#include "IOHandler.hpp"
#include "Block.hpp"
#include "../filesystem/HashDatabase.hpp"
#include "../Logger.hpp"

namespace Lecista {

class Peer : public boost::enable_shared_from_this<Peer>
{
public:
	typedef boost::shared_ptr<Peer> SharedPtr;
	enum class Command : char
	{
		Download = 'D',
		Upload = 'U',

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

	void download(Hash::SharedPtr const& fileHash, uint32_t block);
	void sendBlock(Hash::SharedPtr const& fileHash, uint32_t block);

private:
	void listen();
	void on_read(boost::system::error_code const& ec, size_t bytes);
	void on_writeSent(
		boost::shared_array<char> buffer,
		boost::system::error_code const& ec,
		boost::function<void()> callback);
	void on_write(boost::shared_array<char> b, boost::system::error_code const& ec, uintmax_t start, uintmax_t end);
	void on_download(size_t bytes);
	void readBytes(size_t bytes = 0);
	void disconnect();

	boost::function<void()> m_onClose;
	IOHandler& m_io;
	HashDatabase& m_db;
	Block m_block;
	char m_readBuffer[500 * 1000]; // 500 kB
	boost::asio::ip::tcp::socket m_socket;
	HashDatabase::FileEntry::SharedPtr m_file;
	std::ifstream* m_fh;

	State m_state;
	bool m_parsedArgs;
	size_t m_uploaded;
	size_t m_downloaded;
	unsigned int m_uploadBandwidth;
	unsigned int m_downloadBandwidth;
};

}
