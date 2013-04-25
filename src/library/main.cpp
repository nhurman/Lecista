#include <iostream>

#include "filesystem/Config.hpp"
#include "filesystem/HashDatabase.hpp"
#include "filesystem/DirectoryExplorer.hpp"
#include "filesystem/CompleteFile.hpp"
#include "network/IOHandler.hpp"
#include "network/MulticastHandler.hpp"
#include "network/UnicastHandler.hpp"

using namespace Lecista;

int main()
{
	Config config;
	HashDatabase db;
	DirectoryExplorer explorer(db, config);

	IOHandler io;
	//MulticastHandler mcast(io, db);

	//explorer.addDirectory("../src");
	explorer.addDirectory("/tmp/share");
	//explorer.listDirectories();
	db.list();

	UnicastHandler ucast(io, db);

	//TODO getByHash

	std::cin.get();

	return 0;
}
