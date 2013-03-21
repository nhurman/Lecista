#include "Hash.hpp"

namespace Lecista {

Hash::Hash()
{
	m_hash = new unsigned char[SIZE];
}

Hash::Hash(Hash const& other)
{
	*this = other;
}

Hash::~Hash()
{
	delete[] m_hash;
}

Hash& Hash::operator=(Hash const& other)
{
	std::memcpy(m_hash, other.m_hash, SIZE);
	return *this;
}

Hash& Hash::operator=(std::string const& s)
{
	assert(s.size() >= SIZE);
	std::memcpy(m_hash, s.c_str(), SIZE);
	return *this;
}

unsigned char* Hash::data() const
{
	return m_hash;
}

}