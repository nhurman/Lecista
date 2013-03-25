#pragma once

#include <string>
#include <sstream>
#include <boost/filesystem.hpp>
#include <leveldb/db.h>
#include "HashTree.hpp"

namespace Lecista {

class HashDatabase
{
public:
	class File
	{
	friend class HashDatabase;
	public:
		typedef boost::shared_ptr<File> SharedPtr;

		File(HashTree *t) : m_tree(t) {}
		~File() { delete m_tree; }

		std::string filename() { return m_filename; }
		time_t lastWrite() { return m_lastWrite; }
		HashTree const* tree() { return m_tree; }

	private:
		std::string m_filename;
		time_t m_lastWrite;
		HashTree *m_tree;
	};

	HashDatabase(std::string const& database = "hash_database");
	~HashDatabase();

	void addFile(std::string const& filename);
	File::SharedPtr getFile(std::string const& filename);

private:
	leveldb::DB *m_db;

	unsigned int serialize(std::string const& filename, char*& out) const;
	File::SharedPtr unserialize(char const* data, unsigned int size);
};

}