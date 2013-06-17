#pragma once

#include <cstring>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

namespace Lecista {

class IOHandler
{
public:
	IOHandler() : m_thread(0) {}
	~IOHandler() { stop(); }
	boost::asio::io_service& io() { return m_io; }

	void start()
	{
		if (0 == m_thread) {
			m_thread = new boost::thread(boost::bind(&boost::asio::io_service::run, &m_io));
		}
	}

	void stop()
	{
		if (0 != m_thread) {
			m_io.stop();
			if (!m_thread->try_join_for(boost::chrono::milliseconds(500))) {
				m_thread->interrupt();
			}

			delete m_thread;
			m_thread = 0;
		}
	}

private:
	boost::asio::io_service m_io;
	boost::thread* m_thread;
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
