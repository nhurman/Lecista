#pragma once

#include <cstring>
#include <cassert>
#include <string>
#include <fstream>
#include <openssl/sha.h>
#include "Hash.hpp"

namespace Lecista {

class Hasher
{
private:
	SHA_CTX m_context;

public:
	Hasher();
	void reset();

	void update(std::string const& data);
	void update(const void *data, unsigned long length);
	Hash::SharedPtr getHash();

	static Hash::SharedPtr hashFile(const char *filename);
	static Hash::SharedPtr hashString(const char *string, int size);
};

}