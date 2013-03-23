#include "DirectoryExplorer.hpp"

namespace Lecista {

namespace fs = boost::filesystem;

DirectoryExplorer::DirectoryExplorer()
{

}

void DirectoryExplorer::hashDirectory(std::string path)
{
	for (fs::recursive_directory_iterator end, i(path); i != end; ++i) {
		std::string path = fs::canonical(i->path()).string();
		if (fs::is_directory(path)) {
			continue;
		}

		std::cout << path;
		HashTree tree(path);
		std::cout << " " << std::hex << int(tree.getRootHash()->data()[0]) << std::endl;

		break;
	}


	/*for (deque<string>::iterator i = list.begin(); i != list.end(); i++) {
		cout << *i << endl;
	}*/
}

}