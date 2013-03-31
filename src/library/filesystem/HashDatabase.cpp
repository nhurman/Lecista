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

bool HashDatabase::exists(std::string const& filename)
{
	std::string value;
	leveldb::Status s = m_db->Get(leveldb::ReadOptions(), filename, &value);
	return s.ok();
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

HashDatabase::File::SharedPtr HashDatabase::unserialize(char const* data, unsigned int size, bool tree)
{
	File::SharedPtr file(new File(new HashTree()));
	unsigned int s_lastWrite = sizeof(file->m_lastWrite);

	memcpy(&(file->m_lastWrite), data, s_lastWrite);
	if (tree) {
		file->m_tree->unserialize(data + s_lastWrite, size - s_lastWrite);
	}

	return file;
}

HashDatabase::File::SharedPtr HashDatabase::unserialize(leveldb::Slice const& slice, bool tree)
{
	return unserialize(slice.data(), slice.size(), tree);
}

void HashDatabase::delDirectory(std::string path)
{
	assert(path.length() > 0);

	boost::filesystem::path slash("/");
	std::string preferredSlash = slash.make_preferred().native();

	if (path[path.length() - 1] != preferredSlash[0])
		path += preferredSlash[0];

	auto it = m_db->NewIterator(leveldb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		char const* key = it->key().data();
		bool found = true;
		for (int i = 0; i < path.length() && i < it->key().size(); ++i) {
			if (path[i] != key[i]) {
				found = false;
				break;
			}
		}

		if (found && it->key().size() >= path.length()) {
			m_db->Delete(leveldb::WriteOptions(), it->key());
		}
	}
}

void HashDatabase::rehash()
{
	auto it = m_db->NewIterator(leveldb::ReadOptions());
	time_t lastWrite;

	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		HashDatabase::File::SharedPtr file = unserialize(it->value(), false);
		std::string filename = it->key().ToString();

		try {
			lastWrite = boost::filesystem::last_write_time(filename);

			if (lastWrite != file->lastWrite()) {
				// Update hash
				addFile(filename);
			}
		} catch (boost::filesystem::filesystem_error& e) {
			// File no longer exists, remove it from the index
			m_db->Delete(leveldb::WriteOptions(), it->key());
		}
	}
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
