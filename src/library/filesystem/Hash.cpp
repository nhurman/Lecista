#include "Hash.hpp"

namespace Lecista {

Hash::Hash()
{
	m_hash = new char[SIZE];
	std::memset(m_hash, 0, SIZE);
}

Hash::Hash(Hash const& other)
{
	*this = other;
}

Hash::Hash(char const* data)
{
	m_hash = new char[SIZE];
	std::memcpy(m_hash, data, SIZE);
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

bool Hash::operator==(Hash const& other)
{
	return std::memcmp(m_hash, other.m_hash, SIZE) == 0;
}

char* Hash::data() const
{
	return m_hash;
}

std::string Hash::string() const
{
	std::ostringstream out;
	out << std::hex;
	for (int i = 0; i < SIZE; i += sizeof(int)) {
		out << std::setw(sizeof(int)*2) << std::setfill('0')
			<< htonl(*reinterpret_cast<int*>(m_hash + i))
			<< std::setfill(' ');
	}

	out << std::dec;
	return out.str();
}

}
