#include "Peer.hpp"

using namespace boost::asio;

namespace Lecista {

Peer::Peer(IOHandler& io, HashDatabase& db, boost::function<void()> handler)
 : m_onClose(handler), m_io(io), m_db(db), m_socket(io.ioService()), m_state(State::NewlyCreated)
{
	m_state = State::NewlyCreated;
	m_fh = 0;
}

Peer::~Peer()
{
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
	LOG_DEBUG(m_socket.remote_endpoint().address().to_string() << " Connected");

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

	LOG_DEBUG(m_socket.remote_endpoint().address().to_string() << " >> " << bytes << " bytes");

	if (State::Idle == m_state) { // Waiting for a command
		assert(sizeof(Command) == bytes);
		Command c = *reinterpret_cast<Command*>(m_readBuffer);

		if (Command::Download == c) {
			LOG_DEBUG(m_socket.remote_endpoint().address().to_string() << " Command::Download");
			m_state = State::Uploading;

			// char[20] Hash
			// uint32_t Block
			m_parsedArgs = false;
			readBytes(Hash::SIZE + sizeof(uint32_t));
		}
		else {
			disconnect();
			return;
		}
	}
	else if (State::Uploading == m_state && !m_parsedArgs) {
		assert(Hash::SIZE + sizeof(uint32_t) == bytes);
		Hash::SharedPtr h(new Hash(m_readBuffer));
		uint32_t blockId = ntohl(*reinterpret_cast<uint32_t*>(m_readBuffer + Hash::SIZE));
		m_parsedArgs = true;

		sendBlock(h, blockId);
	}

	if (State::Uploading == m_state) {

	}
	else if (State::Downloading == m_state) {
		on_download(bytes);
	}
}

void Peer::on_download(size_t bytes)
{
	assert(State::Downloading == m_state);

	size_t toRead = 0;
	if (m_block.download(m_readBuffer, bytes, toRead)) {
		readBytes(toRead);
	}
	else {
		m_state = State::Idle;
		readBytes(sizeof(Command));
	}
}

void Peer::sendBlock(Hash::SharedPtr const& fileHash, uint32_t block)
{
	assert(State::Uploading == m_state);

	m_file = m_db.find(fileHash);
	LOG_DEBUG(m_socket.remote_endpoint().address().to_string() << " Sending \"" << m_file->filename() << "\"~" << block);


	// uint32_t   NbHash
	// char[20]   Hash (NbHash times)
	// uint32_t   Size
	// char[Size] Data

	std::vector<Hash::SharedPtr> hashes = m_file->file()->hashList(block);
	unsigned int const headerSize =
		sizeof(uint32_t) + Hash::SIZE * hashes.size() + sizeof(uint32_t);
	boost::shared_array<char> buffer(new char[headerSize]);
	char *b = buffer.get();
	*reinterpret_cast<uint32_t*>(b) = htonl(hashes.size());
	b += sizeof(uint32_t);

	for (int i = 0; i < hashes.size(); ++i) {
		std::memcpy(b, hashes[i]->data(), Hash::SIZE);
		b += Hash::SIZE;
	}

	if (m_fh) {
		delete m_fh;
	}

	m_fh = new std::ifstream(m_file->filename(), std::ios::binary);
	assert(m_fh->is_open());

	uintmax_t startIndex = block * File::BLOCK_SIZE;
	uintmax_t endIndex = std::min(m_file->file()->filesize(), startIndex + File::BLOCK_SIZE);
	assert(startIndex < m_file->file()->filesize());

	*reinterpret_cast<uint32_t*>(b) = htonl(static_cast<uint32_t>(endIndex - startIndex));

	boost::function<void()> callback =
		boost::bind(&Peer::on_write, shared_from_this(),
			buffer,
			boost::system::error_code(),
			startIndex,
			endIndex);

	boost::asio::async_write(m_socket, boost::asio::buffer(buffer.get(), headerSize),
		boost::bind(&Peer::on_writeSent, shared_from_this(),
			buffer, boost::asio::placeholders::error, callback));
}

void Peer::download(Hash::SharedPtr const& fileHash, uint32_t block)
{
	LOG_DEBUG("Downloading " << fileHash->string() << "~" << block);
	m_state = State::Downloading;
	m_block.rootHash(fileHash);

	// Command  cmd
	// char[20] Hash
	// uint32_t Block
	unsigned int const packetSize = sizeof(Command) + 20 + sizeof(uint32_t);
	boost::shared_array<char> buffer(new char[packetSize]);
	char *b = buffer.get();

	*b = static_cast<char>(Command::Download); ++b;
	std::memcpy(b, fileHash->data(), Hash::SIZE); b += Hash::SIZE;
	*reinterpret_cast<uint32_t*>(b) = htonl(block);

	boost::function<void()> callback =
		boost::bind(&Peer::on_read, shared_from_this(),
			boost::system::error_code(), 0);

	boost::asio::async_write(m_socket, boost::asio::buffer(buffer.get(), packetSize),
		boost::bind(&Peer::on_writeSent, shared_from_this(),
			buffer, boost::asio::placeholders::error, callback));
}

void Peer::on_writeSent(
	boost::shared_array<char> buffer,
	boost::system::error_code const& ec,
	boost::function<void()> callback)
{
	callback();
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
		m_fh->read(buffer.get(), BufferSize);
		boost::asio::async_write(m_socket, boost::asio::buffer(buffer.get(), static_cast<size_t>(m_fh->gcount())),
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
