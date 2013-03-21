#include <iostream>

#include "HashTree.hpp"

using namespace Lecista;

int main()
{
	std::deque<std::vector<Hash::SharedPtr> > tree = HashTree::hashFile("/tmp/hello");

	for (std::deque<std::vector<Hash::SharedPtr> >::iterator i = tree.begin(); i != tree.end(); i++) {
		std::cout << "--- Level contains " << (*i).size() << " hashes" << std::endl;

		for (int j = 0; j < (*i).size(); j++) {
			std::cout << "  ";

			unsigned char *data = (*i)[j]->data();
			for (int k = 0; k < 20; k++) {
				std::cout << std::hex << (int)data[k];
			}

			std::cout << std::endl;
		}
	}

	return 0;
}