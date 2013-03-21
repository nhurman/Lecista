#include "HashTree.hpp"

namespace Lecista {

using namespace std;

HashTree::HashTree(string filename)
{
	m_filename = filename;
}

bool HashTree::hashBase()
{
	tree.clear();

	ifstream fh(m_filename.c_str(), ios::binary);
	if (!fh.is_open()) {
		return false;
	}
	
	Hasher hasher;

	fh.seekg(0, fh.end);
	int filesize = fh.tellg();
	int blocks = ceil(filesize / (float)BLOCK_SIZE);

	vector<Hash::SharedPtr> hashList(blocks);
	char buffer[1000 * 1000]; // 1Mo
	unsigned long toRead, read;

	fh.seekg(0, fh.beg);
	for (int i = 0; i < blocks && !fh.eof(); i++) {
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

	tree.push_front(hashList);
	return true;
}


unsigned int HashTree::hashLevel()
{
	assert(tree.size() > 0);
	const vector<Hash::SharedPtr> &blocks = tree.front();

	int size = ceil(blocks.size() / 2.);
	int bsize = sizeof blocks[0];
	vector<Hash::SharedPtr> hashList(size);
	Hasher hasher;

	for (int i = 0; i < size; i++) {
		if (i*2+1 < blocks.size()) {
			hasher.reset();
			hasher.update(blocks[i*2]->data(), bsize);
			hasher.update(blocks[i*2+1]->data(), bsize);

			hashList[i] = hasher.getHash();
		}
		else {
			hashList[i] = blocks[i*2];
		}
	}

	tree.push_front(hashList);
	return hashList.size();
}

deque<vector<Hash::SharedPtr> > HashTree::hash()
{
	hashBase();
	while (hashLevel() > 1);
	return tree;
}

deque<vector<Hash::SharedPtr> > HashTree::hashFile(std::string filename)
{
	HashTree ht(filename);
	return ht.hash();
}

}