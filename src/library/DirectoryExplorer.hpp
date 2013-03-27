#pragma once

#include <string>
#include <map>
#include <deque>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <leveldb/db.h>
#include <boost/filesystem.hpp>

#include "Config.hpp"
#include "HashDatabase.hpp"

namespace Lecista {

class DirectoryExplorer
{
public:
	DirectoryExplorer(HashDatabase& db, Config& config);
	template<class T> static std::string formatSize(T size);
	Config::Directory addDirectory(std::string path, bool rescan = false);
	void delDirectory(std::string path);
	std::map<std::string, Config::Directory> const& listDirectories() const;
	void rescan();

private:
	HashDatabase& m_db;
	Config &m_config;
};

}