#include "HashTree.hpp"

namespace Lecista {

using namespace std;

HashTree::HashTree(string filename)
{
	m_filename = filename;
	hashBase();
	while (hashLevel() > 1);
}

void HashTree::hashBase()
{
	m_tree.clear();

	ifstream fh(m_filename.c_str(), ios::binary);
	if (!fh.is_open()) {
		throw FileNotFoundException();
	}
	
	Hasher hasher;

	fh.seekg(0, fh.end);
	int filesize = fh.tellg();
	m_blocks = ceil(filesize / (float)BLOCK_SIZE);

	vector<Hash::SharedPtr> hashList(m_blocks);
	char buffer[1000 * 1000]; // 1 MB
	unsigned long toRead, read;

	fh.seekg(0, fh.beg);
	for (int i = 0; i < m_blocks && !fh.eof(); i++) {
		toRead = BLOCK_SIZE;
		read = 0;
		hasher.reset();

		while (toRead > 0 && !fh.eof()) {
			fh.read(buffer, min(toRead, sizeof buffer));
			read = fh.gcount();
			toRead -= read;
			hasher.update(&buffer[0], read);
		}

		hashList[i] = hasher.getHash();
	}

	m_tree.push_front(hashList);
}


unsigned int HashTree::hashLevel()
{
	if (m_tree.back().size() < 2) {
		return 1;
	}

	const vector<Hash::SharedPtr> &blocks = m_tree.back();

	int size = ceil(blocks.size() / 2.);
	vector<Hash::SharedPtr> hashList(size);
	Hasher hasher;

	for (int i = 0; i < size; i++) {
		if (i*2+1 < blocks.size()) {
			hasher.reset();
			hasher.update(blocks[i*2]);
			hasher.update(blocks[i*2+1]);

			hashList[i] = hasher.getHash();
		}
		else {
			hashList[i] = blocks[i*2];
		}
	}

	m_tree.push_back(hashList);
	return hashList.size();
}

vector<Hash::SharedPtr> HashTree::blockHashList(int index)
{
	vector<Hash::SharedPtr> hashList(m_tree.size() - 1);
	int parent = index, sibling;

	if (m_tree.size() < 2) {
		// Nothing to do, file is contained in one block
	}
	else {
		// Get hashes of sibling and parents' siblings
		int index = 0;
		for (deque<vector<Hash::SharedPtr> >::iterator i = m_tree.begin(); i + 1 != m_tree.end(); i++) {
			sibling = parent + ((parent % 2 == 0) ? 1 : -1);

			if (sibling < i->size()) {
				hashList[index++] = (*i)[sibling];
			}
			else { // Empty hash pointer for orphan branches
				hashList[index++] = Hash::SharedPtr();
			}

			parent = (parent - (parent % 2)) / 2;
		}
	}

	return hashList;
}

Hash::SharedPtr HashTree::getRootHash()
{
	assert(m_tree.back().size() == 1);
	return m_tree.back()[0];
}

std::deque<std::vector<Hash::SharedPtr> > HashTree::getTree()
{
	return m_tree;
}

unsigned int HashTree::getBlocksCount()
{
	return m_blocks;
}

/* Static methods below */

deque<vector<Hash::SharedPtr> > HashTree::hashFile(std::string filename)
{
	HashTree ht(filename);
	return ht.m_tree;
}

bool HashTree::checkBlockHash(
	int index,
	Hash::SharedPtr hash,
	Hash::SharedPtr rootHash,
	vector<Hash::SharedPtr> hashList)
{
	Hasher hasher;
	int parent = index;
	Hash::SharedPtr globalHash = hash;

	for (vector<Hash::SharedPtr>::iterator i = hashList.begin(); i != hashList.end(); i++) {
		if (!*i) {
			// Orphan branch (non-binary tree), keep current hash
		}
		else {
			hasher.reset();

			if (parent % 2 == 0) {
				hasher.update(globalHash);
				hasher.update(*i);
			}
			else {
				hasher.update(*i);
				hasher.update(globalHash);
			}

			globalHash = hasher.getHash();
		}

		parent = (parent - (parent % 2)) / 2;
	}

	return *globalHash == *rootHash;
}

}