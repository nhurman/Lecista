#include "Hasher.hpp"

namespace Lecista {

Hasher::Hasher()
{
	reset();
}

void Hasher::reset()
{
	SHA1_Init(&m_context);
}

void Hasher::update(std::string const& data)
{
	update(data.c_str(), data.size());
}

void Hasher::update(Hash::SharedPtr const& hash)
{
	update(hash->data(), Hash::SIZE);
}

void Hasher::update(const void *data, unsigned long length)
{
	if (length > 0) {
		SHA1_Update(&m_context, reinterpret_cast<unsigned char const*>(data), length);
	}
}

Hash::SharedPtr Hasher::getHash()
{
	Hash::SharedPtr hash(new Hash());

	SHA1_Final((unsigned char*)hash->m_hash, &m_context);
	return hash;
}

Hash::SharedPtr Hasher::hashFile(char const* filename)
{
	std::ifstream fh(filename, std::ios::binary);
	if (!fh.is_open()) {
		return Hash::SharedPtr();
	}

	char buffer[1000 * 1000]; // 1Mo

	Hasher hasher;

	while (!fh.eof()) {
		fh.read(buffer, sizeof buffer);
		hasher.update(&buffer[0], static_cast<unsigned long>(fh.gcount()));
	}

	return hasher.getHash();
}

Hash::SharedPtr Hasher::hashString(char const* data, int size)
{
	Hasher hasher;
	hasher.update(data, size);
	return hasher.getHash();
}

}
