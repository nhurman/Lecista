#pragma once

#include <cmath>
#include <string>
#include <fstream>
#include <vector>
#include <deque>
#include <boost/filesystem.hpp>

#include "Hash.hpp"
#include "Hasher.hpp"

namespace Lecista {
	
class HashTree
{
public:
	HashTree();
	HashTree(std::string const& filename);
	void hashFile(std::string const& filename);

	std::vector<Hash::SharedPtr> blockHashList(int index);
	Hash::SharedPtr rootHash() const;
	std::deque<std::vector<Hash::SharedPtr>> tree() const { return m_tree; };
	uintmax_t filesize() const { return m_filesize; }

	unsigned int getSerializedSize() const;
	unsigned int serialize(char*& out) const;
	bool unserialize(char const* data, unsigned int size);

	static bool checkBlockHash(
		int index,
		Hash::SharedPtr hash,
		Hash::SharedPtr rootHash,
		std::vector<Hash::SharedPtr> const& hashList);

private:
	static const int BLOCK_SIZE = 16 * 1000 * 1000; // 16 MB
	
	std::string m_filename;
	uintmax_t m_filesize;
	std::deque<std::vector<Hash::SharedPtr>> m_tree;
	
	void hashBase(std::string const& filename);
	unsigned int hashLevel();
};

}

/* Sample hash tree

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