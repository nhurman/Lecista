#pragma once

#include <string>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <leveldb/db.h>
#include "HashTree.hpp"

namespace Lecista {

class HashDatabase
{
public:
	struct File
	{
		std::string filename;
		uintmax_t filesize;
		unsigned int last_write_time;
		HashTree *tree;
	};

	HashDatabase(std::string const& database = "hash_database");
	~HashDatabase();

	void addFile(std::string const& filename);
	void write(File file);

private:
	leveldb::DB *m_db;
};

}