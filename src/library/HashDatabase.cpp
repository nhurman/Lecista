#include "HashDatabase.hpp"

namespace Lecista {

HashDatabase::HashDatabase(std::string const& database)
{
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, database, &m_db);
	assert(status.ok());
}

HashDatabase::~HashDatabase()
{
	delete m_db;
}

void HashDatabase::addFile(std::string const& filename)
{
	char *out = 0;
	unsigned int size = serialize(filename, out);

	leveldb::Slice value(out, size);
	leveldb::Status s = m_db->Put(leveldb::WriteOptions(), filename, value);
	assert(s.ok());

	delete[] out;
}

HashDatabase::File::SharedPtr HashDatabase::getFile(std::string const& filename)
{
	std::string data;
	leveldb::Status s = m_db->Get(leveldb::ReadOptions(), filename, &data);
	assert(s.ok());

	HashDatabase::File::SharedPtr file = unserialize(data.c_str(), data.length());
	file->m_filename = filename;
	return file;
}

unsigned int HashDatabase::serialize(std::string const& filename, char*& out) const
{
	time_t lastWrite = boost::filesystem::last_write_time(filename);
	HashTree tree(filename);

	unsigned int s_lastWrite = sizeof lastWrite;
	unsigned int s_tree = tree.getSerializedSize();
	unsigned int size = s_lastWrite + s_tree;
	
	out = new char[size];
	std::memcpy(out, &lastWrite, s_lastWrite);

	char *treeBeginning = out + s_lastWrite;
	tree.serialize(treeBeginning);

	return size;
}

HashDatabase::File::SharedPtr HashDatabase::unserialize(char const* data, unsigned int size)
{
	File::SharedPtr file(new File(new HashTree()));
	unsigned int s_lastWrite = sizeof(file->m_lastWrite);

	memcpy(&(file->m_lastWrite), data, s_lastWrite);
	file->m_tree->unserialize(data + s_lastWrite, size - s_lastWrite);

	return file;
}

void HashDatabase::list()
{
	auto it = m_db->NewIterator(leveldb::ReadOptions());
	
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		std::cout << it->key().ToString() << std::endl;
	}

	assert(it->status().ok());
	delete it;
}

}
