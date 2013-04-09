#pragma once

#include <cstring>
#include <openssl/sha.h>
#include <boost/shared_ptr.hpp>

namespace Lecista {

//! Hash representation (currently SHA1).
class Hash
{
friend class Hasher;
private:
	char *m_hash;

public:
	//! Use shared pointers to handle memory.
	typedef boost::shared_ptr<Hash> SharedPtr;

	//! Digest size
	static const unsigned int SIZE = SHA_DIGEST_LENGTH;

	//! Constructs an empty hash.
	Hash();

	//! Copy constructor.
	/*!
	  \param other
	*/
	Hash(Hash const& other);

	//! Constructs a hash object from a char table.
	/*!
	  \param data Hash digest. Must be at least SIZE bytes.
	*/
	Hash(char const* data);

	~Hash();

	//! Copy constructor.
	/*!
	  \param other Other hash to opy digest from
	  \return Reference on the current object
	*/
	Hash& operator=(Hash const& other);

	/*! Create a hash from a string.
	  \param s Hash digest, must be at least SIZE bytes
	  \return Reference on the current object
	*/
	Hash& operator=(std::string const& s);

	/*! Compare digests.
	  \param other Other hash to compare with
	  \return True if both hashes are equal
	*/
	bool operator==(Hash const& other);

	//! Return a pointer to the raw data.
	/*!
	  \return Hash digest
	*/
	char* data() const;
};

}
