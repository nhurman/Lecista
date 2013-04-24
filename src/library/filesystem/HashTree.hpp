#pragma once

#include <cmath>
#include <string>
#include <fstream>
#include <vector>
#include <deque>
#include <boost/filesystem.hpp>

#include "Hash.hpp"
#include "Hasher.hpp"
#include "../Logger.hpp"

namespace Lecista {

//! Hash tree calculation and checking
/*! ## Sample hash tree

Given the hashes L0-4, L0-5, L1-3, L2-0, L3-1 and
L4-0 (the root hash) we can check wether or not L0-4
is valid by computing and comparing the root hash.

	File  XXX XXX XXX XXX XXX XXX XXX XXX XXX XX
	L0     0   1   2   3  [4]  5*  6   7   8   9
	       |___|   |___|   |___|   |___|   |___|
	         |       |       |       |       |
	L1       0       1       2       3*      4
	         |_______|       |_______|       |
	             |               |           |
	L2           0*              1           2
	             |_______________|           |
	                     |                   |
	L3                   0                   1*
	                     |___________________|
	                               |
	L4                             0
*/
class HashTree
{
public:
	static const int BLOCK_SIZE = 16 * 1000 * 1000; // 16 MB

	HashTree();

	//! Hash a file
	/*!
	  \param filename
	*/
	HashTree(std::string const& filename);

	//! Hash a file
	/*!
	  \param filename
	*/
	void hashFile(std::string const& filename);

	//! Gather hash nodes required to check a block
	/*!
	  \param index Block index
	  \return Hash digests list
	*/
	std::vector<Hash::SharedPtr> blockHashList(unsigned int index) const;

	//! Get the root hash for the current file
	/*!
	  \return Root hash
	*/
	Hash::SharedPtr rootHash() const;

	//! Fetch the entire tree
	/*!
	  \return Hash tree
	*/
	std::deque<std::vector<Hash::SharedPtr>> tree() const { return m_tree; };

	//! Get the current file's size
	/*!
	  \return File size
	*/
	uintmax_t filesize() const { return m_filesize; }

	//! Calculate the length of the serialized version of the tree
	/*!
	  \return Bytes required to serialize the tree
	*/
	unsigned int getSerializedSize() const;

	//! Return a byte representation of the tree
	/*!
	  \param out Where to write the byte array to. If null, memory will be allocated.
	  \return Size of the byte array
	*/
	unsigned int serialize(char*& out) const;

	//! Take a byte array and repopulate the tree with its contents
	/*!
	  \param data Byte array
	  \param size Byte array length
	  \return True if succeded
	*/
	bool unserialize(char const* data, unsigned int size);

	//! Check the integrity of a block
	/*!
	  \param index Block index
	  \param hash Block hash
	  \param rootHash Root hash of the file
	  \param hashList Hash list as returned by HashTree::blockHashList
	  \return True if the block is valid
	*/
	static bool checkBlockHash(
		unsigned int index,
		Hash::SharedPtr hash,
		Hash::SharedPtr rootHash,
		std::vector<Hash::SharedPtr> const& hashList);

private:
	std::string m_filename;
	uintmax_t m_filesize;
	std::deque<std::vector<Hash::SharedPtr>> m_tree;

	void hashBase(std::string const& filename);
	unsigned int hashLevel();
};

}
