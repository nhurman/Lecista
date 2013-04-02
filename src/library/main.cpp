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
	//MulticastHandler mcast(io);

	explorer.addDirectory("/tmp/structure");
	explorer.listDirectories();


	boost::shared_ptr<std::deque<HashDatabase::File::SharedPtr>> matches = db.search("Hash");
	for (auto i = matches->begin(); i != matches->end(); ++i)
		LOG_DEBUG("match: " << boost::filesystem::path((*i)->filename()).filename().string());


	//TODO getByHash

	//std::cin.get();

	return 0;
}
