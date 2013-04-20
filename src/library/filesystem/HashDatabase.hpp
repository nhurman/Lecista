#pragma once

#include <string>
#include <sstream>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <leveldb/db.h>
#include "HashTree.hpp"
#include "../Logger.hpp"

namespace Lecista {

//! Stores hashes of shared files LevelDB database.
class HashDatabase
{
public:
	//! File representation
	class File
	{
	friend class HashDatabase;
	public:
		//! Use shared pointers to handle memory
		typedef boost::shared_ptr<File> SharedPtr;

		~File() { delete m_tree; }

		std::string filename() { return m_filename; }
		time_t lastWrite() { return m_lastWrite; }
		HashTree const* tree() { return m_tree; }

	private:
		//! Constructs the object and initializes the hash tree
		/*!
		  \param t Hash tree
		*/
		File(HashTree *t) : m_tree(t) {}

		std::string m_filename;
		time_t m_lastWrite;
		HashTree *m_tree;
	};

	class PartialFile : public File
	{
	friend class HashDatabase;
	public:

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
	File::SharedPtr getFile(std::string const& filename);

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
	boost::shared_ptr<std::deque<File::SharedPtr>> search(std::string name);

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
	File::SharedPtr unserialize(char const* data, unsigned int size, bool tree = true);
	File::SharedPtr unserialize(leveldb::Slice const& slice, bool tree = true);
};

}
