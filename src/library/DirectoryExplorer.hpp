#pragma once

#include <string>
#include <deque>
#include <algorithm>
#include <leveldb/db.h>
#include <boost/filesystem.hpp>
#include "HashDatabase.hpp"

namespace Lecista {

class DirectoryExplorer
{
public:
	struct Directory
	{
		std::string name;
	};

	DirectoryExplorer(HashDatabase& db);
	void addDirectory(std::string path);
	std::deque<std::string> listDirectories();

private:
	HashDatabase& m_db;
	unsigned int m_hashCount;
};

}