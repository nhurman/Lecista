#include "HashDatabase.hpp"

namespace Lecista {

HashDatabase::HashDatabase(std::string const& database)
{
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, database, &m_db);
	assert(status.ok());

	addFile("/usr/lib/flashplugin-installer/libflashplayer.so");
}

HashDatabase::~HashDatabase()
{
	delete m_db;
}

void HashDatabase::addFile(std::string const& filename)
{
	/*HashTree tree(filename);

	char *d = 0;
	int size = tree.serialize(d);

	delete[] d;*/
}

void HashDatabase::write(File file)
{

}

}