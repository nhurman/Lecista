#include "Block.hpp"

namespace Lecista {

Block::Block()
{
	init();
}

Block::~Block()
{

}

void Block::rootHash(Hash::SharedPtr const& hash)
{
	m_rootHash = hash;
}

void Block::init()
{
	m_size = 0;
	m_state = NewlyCreated;
	m_readBytes = 0;
	m_readHashes = 0;
}

bool Block::download(char const* buffer, size_t bytes, size_t& toRead)
{
	// uint32_t   NbHash
	// char[20]   Hash (NbHash times)
	// uint32_t   Size

	switch (m_state) {
		case NewlyCreated: {
			assert(0 == bytes);
			init();
			m_state = Block::NbHash;
			toRead = sizeof(uint32_t);
			break;
		}

		case NbHash: {
			assert(sizeof(uint32_t) == bytes);
			uint32_t nbHash = ntohl(*reinterpret_cast<uint32_t const*>(buffer));
			m_hashList = std::vector<Hash::SharedPtr>(nbHash);

			if (nbHash > 0) {
				m_state = Block::HashList;
				toRead = Hash::SIZE;
			}
			else {
				m_state = Block::Size;
				toRead = sizeof(uint32_t);
			}
			break;
		}

		case HashList: {
			assert(Hash::SIZE == bytes);
			Hash::SharedPtr hash(new Hash(buffer));
			m_hashList[m_readHashes++] = hash;

			if (m_hashList.size() == m_readHashes) {
				m_state = Block::Size;
				toRead = sizeof(uint32_t);
			}
			else {
				toRead = Hash::SIZE;
			}
			break;
		}

		case Size: {
			assert(sizeof m_size == bytes);
			m_size = ntohl(*reinterpret_cast<uint32_t const*>(buffer));
			m_state = Block::Data;
			toRead = 0;
			break;
		}

		case Data: {
			size_t maxRead = std::min(bytes, (size_t)(m_size - m_readBytes));
			//!TODO Save data
			m_readBytes += maxRead;
			m_hasher.update(buffer, maxRead);
			toRead = 0;

			if (m_readBytes == m_size) {
				m_hash = m_hasher.getHash();

				//!TODO Check hash
				//file.checkBlock(0, m_hash, m_hashList);

				m_state = NewlyCreated;
				return false;
			}
			break;
		}

		default:
			break;
	}

	return true;
}

}
