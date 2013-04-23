#include "Peer.hpp"

using namespace boost::asio;

namespace Lecista {

Peer::Peer(IOHandler& io, HashDatabase& db, boost::function<void()> handler)
 : m_onClose(handler), m_io(io), m_db(db), m_socket(io.ioService()), m_state(State::NewlyCreated)
{
	LOG_DEBUG("Constructor");
	m_state = State::NewlyCreated;
	m_fh = 0;
}

Peer::~Peer()
{
	LOG_DEBUG("Destructor");
	if (m_fh) {
		delete m_fh;
	}
}

void Peer::disconnect()
{
	if (State::Closed != m_state) {
		LOG_DEBUG("Disconnect" << (m_state > State::NewlyCreated ? " " + m_socket.remote_endpoint().address().to_string() : ""));
		m_socket.close();
		m_state = State::Closed;
		m_io.ioService().post(m_onClose);
	}
}

void Peer::init()
{
	LOG_DEBUG("initiate() with " << m_socket.remote_endpoint().address().to_string());

	m_state = State::Idle;
	readBytes(sizeof(Command));
}

void Peer::readBytes(size_t bytes)
{
	if (0 == bytes) {
		m_socket.async_read_some(
			boost::asio::buffer(m_readBuffer, sizeof m_readBuffer),
			boost::bind(&Peer::on_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}
	else {
		boost::asio::async_read(m_socket,
			boost::asio::buffer(m_readBuffer, std::min(bytes, sizeof m_readBuffer)),
			boost::bind(&Peer::on_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}
}

void Peer::on_read(boost::system::error_code const& ec, size_t bytes)
{
	if (ec) {
		// Shutting down ?
		if (boost::asio::error::operation_aborted != ec &&
			boost::asio::error::bad_descriptor != ec) {
			LOG_DEBUG("Error in on_read: " << ec.value() << ": " << ec.message());
			disconnect();
		}

		return;
	}

	LOG_DEBUG("Got " << bytes << " bytes");

	if (State::Idle == m_state) { // Waiting for a command
		assert(sizeof(Command) == bytes);
		Command c = *reinterpret_cast<Command*>(m_readBuffer);

		if (Command::DownloadBlock == c) {
			LOG_DEBUG("Command::DownloadBlock");

			HashDatabase::File::SharedPtr f = m_db.getFile(
				"/home/nicolas/Prog/C++/Lecista/src/library/main.cpp");
			Hash::SharedPtr h = f->tree()->rootHash();
			sendBlock(h, 0);
		}
		else if (Command::UploadBlock == c) {
			LOG_DEBUG("Command::UploadBlock");
			m_state = State::Downloading;
		}
		else {
			disconnect();
			return;
		}
	}

	if (State::Uploading == m_state) {

	}
	else if (State::Downloading == m_state) {
		readBytes();
	}
}

void Peer::sendBlock(Hash::SharedPtr fileHash, unsigned int block)
{
	assert(State::Idle == m_state);
	m_state = State::Uploading;

	m_file = m_db.find(fileHash);

	if (m_fh) {
		delete m_fh;
	}

	m_fh = new std::ifstream(m_file->filename(), std::ios::binary);
	assert(m_fh->is_open());

	uintmax_t startIndex = block * HashTree::BLOCK_SIZE;
	uintmax_t endIndex = std::min(m_file->tree()->filesize(), startIndex + HashTree::BLOCK_SIZE);
	assert(startIndex < m_file->tree()->filesize());

	on_write(boost::shared_array<char>(), boost::system::error_code(), startIndex, endIndex);
}

void Peer::on_write(boost::shared_array<char> b, boost::system::error_code const& ec, uintmax_t start, uintmax_t end)
{
	if (ec) {
		// Shutting down ?
		if (boost::asio::error::operation_aborted != ec &&
			boost::asio::error::bad_descriptor != ec) {
			LOG_DEBUG("Error in on_write: " << ec.value() << ": " << ec.message());
			disconnect();
		}

		return;
	}

	if (State::Uploading != m_state) {
		return;
	}

	unsigned int const BufferSize = 500 * 1000; // 500kB
	boost::shared_array<char> buffer(new char[BufferSize]);
	m_fh->seekg(start, m_fh->beg);

	if (!m_fh->eof() && m_fh->tellg() < end) {
		LOG_DEBUG("START=" << start << " END=" << end);
		m_fh->read(buffer.get(), BufferSize);
		m_socket.async_send(boost::asio::buffer(buffer.get(), m_fh->gcount()),
			boost::bind(&Peer::on_write, shared_from_this(), buffer,
				boost::asio::placeholders::error,
				start + m_fh->gcount(), end));
	}
	else {
		m_state = State::Idle;
		readBytes(sizeof(Command));
	}
}


}
