#include "HashTree.hpp"

namespace Lecista {

using namespace std;

HashTree::HashTree() : m_filesize(0)
{

}

HashTree::HashTree(string const& filename)
{
	HashTree();
	hashFile(filename);
}

void HashTree::hashFile(string const& filename)
{
	hashBase(filename);
	while (hashLevel() > 1);
}

void HashTree::hashBase(string const& filename)
{
	m_tree.clear();

	ifstream fh(filename.c_str(), ios::binary);
	if (!fh.is_open()) {
		throw boost::filesystem::filesystem_error("Could not open file", filename, boost::system::error_code());
	}

	Hasher hasher;

	m_filesize = boost::filesystem::file_size(filename);
	unsigned int blocks = ceil(m_filesize / (float)BLOCK_SIZE);

	if (0 == blocks) { // Empty file
		blocks = 1;
	}

	vector<Hash::SharedPtr> hashList(blocks);
	char buffer[1000 * 1000]; // 1 MB
	unsigned int toRead, read;

	fh.seekg(0, fh.beg);
	for (unsigned int i = 0; i < blocks && !fh.eof(); ++i) {
		toRead = BLOCK_SIZE;
		read = 0;
		hasher.reset();

		while (toRead > 0 && !fh.eof()) {
			fh.read(buffer, min(toRead, (unsigned int)sizeof buffer));
			read = fh.gcount();

			if (read > 0) {
				toRead -= read;
				hasher.update(&buffer[0], read);
			}
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

	unsigned int size = ceil(blocks.size() / 2.);
	vector<Hash::SharedPtr> hashList(size);
	Hasher hasher;

	for (unsigned int i = 0; i < size; ++i) {
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

vector<Hash::SharedPtr> HashTree::blockHashList(unsigned int index)
{
	vector<Hash::SharedPtr> hashList(m_tree.size() - 1);
	unsigned int parent = index, sibling;

	if (m_tree.size() < 2) {
		// Nothing to do, file is contained in one block
	}
	else {
		// Get hashes of sibling and parents' siblings
		unsigned int index = 0;
		for (auto i = m_tree.begin(); i + 1 != m_tree.end(); ++i) {
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

Hash::SharedPtr HashTree::rootHash() const
{
	assert(m_tree.back().size() == 1);
	return m_tree.back()[0];
}

unsigned int HashTree::getSerializedSize() const
{
	unsigned int
		s_filesize = sizeof m_filesize,
		s_hashlist;

	// How many hashes are contained in this tree ?
	unsigned int lastLevel = ceil(m_filesize / (float)BLOCK_SIZE);
	if (0 == lastLevel) { // Empty file
		lastLevel = 1;
	}

	unsigned int total = lastLevel;
	if (total > 1) {
		total += 1;
	}

	while ((lastLevel = ceil(lastLevel / 2.)) > 1) {
		total += lastLevel;
	}

	s_hashlist = total * Hash::SIZE;
	return s_filesize + s_hashlist;
}

unsigned int HashTree::serialize(char*& out) const
{
	unsigned int i = 0;
	if (out == 0) {
		out = new char[getSerializedSize()];
	}

	memcpy(out + i, &m_filesize, sizeof m_filesize);
	i += sizeof m_filesize;

	for (auto j = m_tree.begin(); j != m_tree.end(); ++j) {
		for (auto k = j->begin(); k != j->end(); ++k) {
			memcpy(out + i, (*k)->data(), Hash::SIZE);
			i += Hash::SIZE;
		}
	}

	return i;
}

bool HashTree::unserialize(char const* data, unsigned int size)
{
	unsigned int i_hashlist  = sizeof m_filesize;
	memcpy(&m_filesize, data, sizeof m_filesize);
	m_tree.clear();

	unsigned int lastLevel = ceil(m_filesize / (float)BLOCK_SIZE);
	if (0 == lastLevel) { // Empty file
		lastLevel = 1;
	}

	for (lastLevel = 2 * lastLevel; lastLevel != 1; ) {
		lastLevel = ceil(lastLevel / 2.);

		vector<Hash::SharedPtr> level(lastLevel);

		for (unsigned int i = 0; i < lastLevel; ++i) {
			level[i] = Hash::SharedPtr(new Hash(data + i_hashlist));
			i_hashlist += Hash::SIZE;
		}

		m_tree.push_back(level);
	}

	return true;
}

/* Static methods below */

bool HashTree::checkBlockHash(
	unsigned int index,
	Hash::SharedPtr hash,
	Hash::SharedPtr rootHash,
	vector<Hash::SharedPtr> const& hashList)
{
	Hasher hasher;
	int parent = index;
	Hash::SharedPtr globalHash = hash;

	for (auto i = hashList.begin(); i != hashList.end(); ++i) {
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
