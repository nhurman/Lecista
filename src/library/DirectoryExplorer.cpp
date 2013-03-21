#include "DirectoryExplorer.hpp"

#include <iostream>
#include <deque>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

namespace Lecista {

DirectoryExplorer::DirectoryExplorer()
{

}

deque<string> DirectoryExplorer::getStructure(string path)
{
	deque<string> list;
	for (fs::recursive_directory_iterator end, i(path); i != end; i++) {
		list.push_back(i->path().string());
	}

	return list;

	/*for (deque<string>::iterator i = list.begin(); i != list.end(); i++) {
		cout << *i << endl;
	}*/
}

}