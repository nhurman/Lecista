#pragma once

#include "../Logger.hpp"
#include "../filesystem/Hash.hpp"
#include "../filesystem/Hasher.hpp"
#include "../filesystem/File.hpp"

namespace Lecista {

class Block
{
public:
	Block();
	~Block();
	void rootHash(Hash::SharedPtr const& hash);
	bool download(char const* data, size_t bytes, size_t& toRead);

private:
	void init();

	uint32_t m_size;
	uint32_t m_readHashes;
	uint32_t m_readBytes;

	std::vector<Hash::SharedPtr> m_hashList;
	Hasher m_hasher;
	Hash::SharedPtr m_hash;
	Hash::SharedPtr m_rootHash;

	enum
	{
		NewlyCreated,
		NbHash,
		HashList,
		Size,
		Data
	} m_state;
};

}
