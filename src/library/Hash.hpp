#pragma once

#include <cstring>
#include <openssl/sha.h>
#include <boost/shared_ptr.hpp>

namespace Lecista {

class Hash
{
friend class Hasher;
private:
	unsigned char *m_hash;

public:
	typedef boost::shared_ptr<Hash> SharedPtr;
	static const unsigned int SIZE = SHA_DIGEST_LENGTH;

	Hash();
	Hash(Hash const& other);
	Hash(const char *data);
	~Hash();

	Hash& operator=(Hash const& other);
	Hash& operator=(std::string const& s);
	bool operator==(Hash const& other);
	unsigned char* data() const;
};

}