#pragma once

#include <cstring>
#include <cassert>
#include <string>
#include <fstream>
#include <openssl/sha.h>
#include "Hash.hpp"

namespace Lecista {

//! Compute hashes
class Hasher
{
private:
	SHA_CTX m_context;

public:
	//! Construct an empty hash
	Hasher();

	//! Start the computation over
	void reset();

	//! Add new data
	/*!
	  \param data
	*/
	void update(std::string const& data);

	//! Add new data
	/*!
	  \param data
	*/
	void update(Hash::SharedPtr const& hash);

	//! Add new data
	/*!
	  \param data Byte array
	  \param length Byte array size
	*/
	void update(const void *data, unsigned long length);

	//! Hash all the data given with calls to update()
	/*!
	  \return Overall hash
	*/
	Hash::SharedPtr getHash();

	//! Hash a file
	/*!
	  \param filename
	  \return File hash
	*/
	static Hash::SharedPtr hashFile(char const* filename);

	//! Hash a byte array
	/*!
	  \param string Byte array
	  \param size Byte array size
	  \return Byte array hash
	*/
	static Hash::SharedPtr hashString(char const* data, int size);
};

}
