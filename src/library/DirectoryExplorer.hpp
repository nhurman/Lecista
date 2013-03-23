#pragma once

#include <string>
#include <deque>
#include <leveldb/db.h>
#include <boost/filesystem.hpp>
#include "HashTree.hpp"

namespace Lecista {

class DirectoryExplorer
{
public:
	struct Directory
	{
		std::string name;
	};

	DirectoryExplorer();
	void hashDirectory(std::string path);

private:
};

}