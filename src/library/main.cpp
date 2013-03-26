#include <iostream>

#include "HashDatabase.hpp"
#include "DirectoryExplorer.hpp"

using namespace Lecista;

int main()
{
	HashDatabase db;
	
	DirectoryExplorer explorer(db);
	explorer.addDirectory("../src/library");
	explorer.listDirectories();

	return 0;
}