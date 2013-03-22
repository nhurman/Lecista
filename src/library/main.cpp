#include <iostream>

#include "HashTree.hpp"

using namespace Lecista;

void printHash(const unsigned char* hash) {
	for (int k = 0; k < 20; k++) {
		std::cout << std::hex << (int)hash[k];
	}
}

void printHashList(std::vector<Hash::SharedPtr> const& hashList)
{
	for (int j = 0; j < hashList.size(); j++) {
		std::cout << "  ";

		if (hashList[j] != 0) {
			unsigned char *data = hashList[j]->data();
			printHash(data);
		}
		else {
			std::cout << "(empty)";
		}

		std::cout << std::endl;
	}
}

int main()
{
	HashTree tree("/tmp/10GB");
	Hash::SharedPtr rootHash = tree.getRootHash();
	std::deque<std::vector<Hash::SharedPtr> > dtree = tree.getTree();

	for (std::deque<std::vector<Hash::SharedPtr> >::iterator i = dtree.begin(); i != dtree.end(); i++) {
		std::cout << "--- Level contains " << (*i).size() << " hashes" << std::endl;
		printHashList(*i);
	}

	std::cout << std::endl << std::endl;

	std::vector<Hash::SharedPtr> hashList = tree.blockHashList(0);
	Hash::SharedPtr blockHash = dtree.front()[0];

	std::cout << "--- Hash list for block #0 (contains " << hashList.size() << " hashes)" << std::endl;
	printHashList(hashList);

	bool isOk = HashTree::checkBlockHash(0, blockHash, rootHash, hashList);
	std::cout << (isOk ? "Ok" : "Corrupted") << std::endl;

	return 0;
}