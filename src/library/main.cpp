#include <iostream>

#include "HashDatabase.hpp"
#include "DirectoryExplorer.hpp"
#include "Config.hpp"

using namespace Lecista;

int main()
{
	Config config;
	HashDatabase db;
	
	DirectoryExplorer explorer(db, config);

	explorer.addDirectory("../src/library");
	
	explorer.listDirectories();
	std::cout << "---" << std::endl;
	db.list();

	return 0;
}