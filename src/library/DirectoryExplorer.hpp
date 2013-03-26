#pragma once

#include <string>
#include <deque>
#include <sstream>
#include <iomanip>
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
		Directory() : size(0), files(0) {}

		std::string name;
		float size;
		unsigned int files;
	};

	DirectoryExplorer(HashDatabase& db);
	template<class T> static std::string formatSize(T size);
	Directory addDirectory(std::string path);
	std::deque<std::string> listDirectories();

private:
	HashDatabase& m_db;
};

}