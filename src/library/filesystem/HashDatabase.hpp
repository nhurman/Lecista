#pragma once

#include <boost/asio.hpp>
#include <string>
#include <sstream>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <leveldb/db.h>
#include "CompleteFile.hpp"
#include "File.hpp"
#include "../Logger.hpp"

namespace Lecista {

//! Stores hashes of shared files in a LevelDB database.
class HashDatabase
{
public:
	//! File representation
	class FileEntry
	{
	friend class HashDatabase;
	public:
		//! Use shared pointers to handle memory
		typedef boost::shared_ptr<FileEntry> SharedPtr;

		~FileEntry() { if (m_file) delete m_file; }

		std::string filename() const { return m_filename; }
		time_t lastWrite() const { return m_lastWrite; }
		File const* file() { return m_file; }

	private:
		FileEntry() : m_file(0) {}

		std::string m_filename;
		time_t m_lastWrite;
		File* m_file;
	};

	/*!
	  \param database Database directory path
	*/
	HashDatabase(std::string const& database = "hashdb");

	~HashDatabase();

	//! Hash a file and add it to the index.
	/*!
	  \param filename File path
	*/
	void addFile(std::string const& filename);

	//! Retrieve a file from the index.
	/*!
	  \param filename File path
	  \return File structure
	*/
	FileEntry::SharedPtr find(std::string const& filename);

	//! Fetch a file with the corresponding hash
	/*!
	  \param hash File hash
	  \return File structure
	*/
	FileEntry::SharedPtr find(Hash::SharedPtr const& hash);

	//! Check if a file is cached in the index.
	/*!
	  \param filename File path
	  \return True if this file's hash is in the index
	*/
	bool exists(std::string const& filename);

	//! Remove a file from the database
	/*!
	  \param fimename
	*/
	void delFile(std::string const& filename);

	//! Remove files belonging to a directory from the index.
	/*!
	  \param path Directory path
	*/
	void delDirectory(std::string path);

	//! Check if the index is in sync with the filesystem. Re-hash modified files and remove deleted files.
	void rehash();

	//! Search for a file in the database
	/*!
	  \param name String to look for in file names
	  \return List of mathing file names
	*/
	boost::shared_ptr<std::deque<FileEntry::SharedPtr>> search(std::string name);

	//! Dump the index
	//! \todo Remove this
	void list();

private:
	enum class Key: unsigned char
	{
		Hash,
		Filename,
		Partial,
		End
	};

	leveldb::DB *m_db;

	unsigned int serialize(std::string const& filename, char*& out, char* rootHash = 0) const;
	FileEntry::SharedPtr unserialize(char const* data, unsigned int size, bool file = true);
	FileEntry::SharedPtr unserialize(leveldb::Slice const& slice, bool file = true);
};

}
