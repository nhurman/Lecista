#pragma once

#include <cstring>
#include <boost/shared_ptr.hpp>

namespace Lecista {

class IOHandler
{
public:
	IOHandler() {}
	boost::asio::io_service& io() { return m_io; }

private:
	boost::asio::io_service m_io;
};

class Hash
{
public:
	Hash(char const* s) { std::memcpy(m_hash, s, 20); }
	Hash(Hash const& other) : Hash(other.m_hash) {}

	bool operator<(Hash const& other) const
	{
		return std::memcmp(m_hash, other.m_hash, 20) < 0;
	}

	bool operator==(Hash const& other) const
	{
		return (std::memcmp(m_hash, other.m_hash, 20) == 0);
	}

	char const* hash() const { return m_hash; }

private:
	char m_hash[20];
};

class File
{
public:
	typedef boost::shared_ptr<File> SharedPtr;

	File(unsigned int size, Hash const& hash) : m_hash(hash) { }
	Hash const& rootHash() const { return m_hash; }

	bool operator==(File const& other) const { return m_hash == other.m_hash; }
	bool operator<(File const& other) const { return m_hash < other.m_hash; }

private:
	Hash m_hash;
};



}
