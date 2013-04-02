#pragma once

#include <string>
#include <map>
#include <deque>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <leveldb/db.h>
#include <boost/filesystem.hpp>
#include "../Logger.hpp"
#include "Config.hpp"
#include "HashDatabase.hpp"

namespace Lecista {

//! Explores directories to hash files and add shares.
class DirectoryExplorer
{
public:
	/*!
	  \param db Hash database handler
	  \param config Configuration handler
	*/
	DirectoryExplorer(HashDatabase& db, Config& config);

	//! Make a size in bytes human-readable using other untis (kB, MB, GB...).
	/*!
	  \param size Size, in bytes
	  \return Formatted size
	*/
	template<class T> static std::string formatSize(T size);

	//! Share a directory and hash all contained files.
	/*!
	  \param path
	  \param rescan If set to true, only add new files to the index. Do no re-hash existing files.
	  \return Directory structure containing the total size and files counts.
	*/
	Config::Directory addDirectory(std::string path, bool rescan = false);

	//! Un-share a directory and remove file hashes from the database.
	/*!
	  \param path
	*/
	void delDirectory(std::string path);

	//! List shared directories.
	/*!
	  \return Shared directories, where the directory name is the index
	*/
	std::map<std::string, Config::Directory> const& listDirectories() const;

	//! Scan shared directories and add new files to the index. Does not check for newer versions.
	void rescan();

private:
	HashDatabase& m_db;
	Config &m_config;
};

}
