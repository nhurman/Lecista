#include <iostream>

#include "filesystem/Config.hpp"
#include "filesystem/HashDatabase.hpp"
#include "filesystem/DirectoryExplorer.hpp"
#include "network/IOHandler.hpp"
#include "network/MulticastHandler.hpp"

using namespace Lecista;

int main()
{
	Config config;
	HashDatabase db;
	DirectoryExplorer explorer(db, config);

	IOHandler io;
	MulticastHandler mcast(io, db);

	explorer.addDirectory("../src");
	explorer.listDirectories();

	//TODO getByHash

	std::cin.get();

	return 0;
}
