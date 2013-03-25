#include <iostream>

#include "HashDatabase.hpp"

using namespace Lecista;

int main()
{
	//HashTree tree("/tmp/10GB");
	//Hash::SharedPtr rootHash = tree.getRootHash();
	
	//DirectoryExplorer explorer;
	//explorer.hashDirectory("../src/library/");

	HashDatabase db;

	//db.addFile("/home/nicolas/Desktop/Maths");
	//db.addFile("/usr/lib/flashplugin-installer/libflashplayer.so");

	std::cout << db.getFile("/home/nicolas/Desktop/Maths")->tree()->filesize() << std::endl;
	std::cout << db.getFile("/usr/lib/flashplugin-installer/libflashplayer.so")->tree()->filesize() << std::endl;

	return 0;
}