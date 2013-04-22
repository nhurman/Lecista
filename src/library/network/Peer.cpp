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
	LOG_DEBUG("Destructor" << (m_state > State::NewlyCreated ? " " + m_socket.remote_endpoint().address().to_string() : ""));
	m_socket.close();
	if (m_fh) {
		delete m_fh;
	}
}

void Peer::init()
{
	LOG_DEBUG("initiate() with " << m_socket.remote_endpoint().address().to_string());

	m_state = State::Idle;
	listen();
}

void Peer::listen()
{
	m_socket.async_read_some(
		boost::asio::buffer(m_readBuffer, sizeof m_readBuffer),
		boost::bind(&Peer::on_read, this,
			boost::asio::placeholders::error,
			placeholders::bytes_transferred));
}

void Peer::on_read(boost::system::error_code const& ec, size_t bytes)
{
	if (ec) {
		LOG_DEBUG("Error " << ec.value() << ": " << ec.message());
		disconnect();
		return;
	}

	LOG_DEBUG("Got " << bytes << " bytes");
	listen();

	HashDatabase::File::SharedPtr f = m_db.getFile(
		"c:/Users\\Nicolas\\Documents\\GitHub\\Lecista\\src\\library\\main.cpp");
	Hash::SharedPtr h = f->tree()->rootHash();
	sendBlock(h, 0);
}

void Peer::disconnect()
{
	if (State::Closed != m_state) {
		LOG_DEBUG("Closing");
		m_state = State::Closed;
		m_io.ioService().post(m_onClose);
	}
}


void Peer::sendBlock(Hash::SharedPtr fileHash, unsigned int block)
{
	assert(State::Idle == m_state);
	m_state = State::SendingBlock;

	m_file = m_db.find(fileHash);

	if (m_fh) {
		delete m_fh;
	}

	m_fh = new std::ifstream(m_file->filename(), std::ios::binary);
	assert(m_fh->is_open());

	uintmax_t startIndex = block * HashTree::BLOCK_SIZE;
	uintmax_t endIndex = std::min(m_file->tree()->filesize(), startIndex + HashTree::BLOCK_SIZE);
	assert(startIndex < m_file->tree()->filesize());

	on_write(boost::system::error_code(), startIndex, endIndex);
}

void Peer::on_write(boost::system::error_code const& ec, uintmax_t start, uintmax_t end)
{
	if (ec) {
		LOG_DEBUG("Error " << ec.value() << ": " << ec.message());
		disconnect();
		return;
	}

	char buffer[500 * 1000]; // 500kB
	m_fh->seekg(start, m_fh->beg);

	if (!m_fh->eof() && m_fh->tellg() < end) {
		LOG_DEBUG("START=" << start << " END=" << end);
		m_fh->read(buffer, sizeof buffer);
		m_socket.async_send(boost::asio::buffer(buffer, m_fh->gcount()),
			boost::bind(&Peer::on_write, this, boost::asio::placeholders::error,
				start + m_fh->gcount(), end));
	}
	else {
		disconnect();
	}
}


}
