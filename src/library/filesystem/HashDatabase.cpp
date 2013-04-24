#include "HashDatabase.hpp"

namespace Lecista {

HashDatabase::HashDatabase(std::string const& database)
{
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, database, &m_db);
	assert(status.ok());

	// Initialize database
	std::string empty;
	m_db->Put(leveldb::WriteOptions(), std::string(1, static_cast<char>(Key::Hash)), empty);
	m_db->Put(leveldb::WriteOptions(), std::string(1, static_cast<char>(Key::Filename)), empty);
	m_db->Put(leveldb::WriteOptions(), std::string(1, static_cast<char>(Key::Partial)), empty);
	m_db->Put(leveldb::WriteOptions(), std::string(1, static_cast<char>(Key::End)), empty);
}

HashDatabase::~HashDatabase()
{
	delete m_db;
}

void HashDatabase::addFile(std::string const& filename)
{
	char *out = 0, rootHash[Hash::SIZE];
	unsigned int size = serialize(filename, out, rootHash);
	std::string hash(rootHash, sizeof rootHash);

	leveldb::Slice value(out, size);
	std::string hashkey = static_cast<char>(Key::Hash) + hash;
	std::string namekey = static_cast<char>(Key::Filename) + filename;

	// Write data
	leveldb::Status s = m_db->Put(leveldb::WriteOptions(), hashkey, value);
	assert(s.ok());

	// Write index
	s = m_db->Put(leveldb::WriteOptions(), namekey, hash);
	assert(s.ok());

	delete[] out;
}

HashDatabase::File::SharedPtr HashDatabase::getFile(std::string const& filename)
{
	std::string data, key = static_cast<char>(Key::Filename) + filename;
	leveldb::Status s = m_db->Get(leveldb::ReadOptions(), key, &data);
	assert(s.ok());

	key = static_cast<char>(Key::Hash) + data;
	s = m_db->Get(leveldb::ReadOptions(), key, &data);
	assert(s.ok());

	HashDatabase::File::SharedPtr file = unserialize(data.c_str(), data.length());
	return file;
}

HashDatabase::File::SharedPtr HashDatabase::find(Hash::SharedPtr const& hash)
{
	std::string data, key = static_cast<char>(Key::Hash) +
		std::string(hash->data(), Hash::SIZE);
	leveldb::Status s = m_db->Get(leveldb::ReadOptions(), key, &data);
	assert(s.ok());

	HashDatabase::File::SharedPtr file = unserialize(data.c_str(), data.length());
	return file;
}

bool HashDatabase::exists(std::string const& filename)
{
	std::string data, key = static_cast<char>(Key::Filename) + filename;
	leveldb::Status s = m_db->Get(leveldb::ReadOptions(), key, &data);
	return s.ok();
}

unsigned int HashDatabase::serialize(std::string const& filename, char*& out, char* rootHash) const
{
	time_t lastWrite = boost::filesystem::last_write_time(filename);
	HashTree tree(filename);
	if (rootHash) {
		std::memcpy(rootHash, tree.rootHash()->data(), Hash::SIZE);
	}

	unsigned int s_filename = filename.length() + 1;
	unsigned int s_lastWrite = sizeof lastWrite;
	unsigned int s_tree = tree.getSerializedSize();
	unsigned int size = s_filename + s_lastWrite + s_tree;

	out = new char[size];
	std::memcpy(out, filename.c_str(), s_filename);
	std::memcpy(out + s_filename, &lastWrite, s_lastWrite);

	char *treeBeginning = out + s_filename + s_lastWrite;
	tree.serialize(treeBeginning);

	return size;
}

HashDatabase::File::SharedPtr HashDatabase::unserialize(char const* data, unsigned int size, bool tree)
{
	File::SharedPtr file(new File(new HashTree()));
	unsigned int s_lastWrite = sizeof(file->m_lastWrite);
	unsigned int s_filename;

	for (s_filename = 0; s_filename < size - s_lastWrite - Hash::SIZE; ++s_filename) {
		if (data[s_filename] == '\0') {
			break;
		}
	}

	++s_filename;
	assert(s_filename < size - s_lastWrite - Hash::SIZE);
	file->m_filename = data;

	memcpy(&(file->m_lastWrite), data + s_filename, s_lastWrite);
	if (tree) {
		file->m_tree->unserialize(data + s_filename + s_lastWrite, size - s_lastWrite - s_filename);
	}

	return file;
}

HashDatabase::File::SharedPtr HashDatabase::unserialize(leveldb::Slice const& slice, bool tree)
{
	return unserialize(slice.data(), slice.size(), tree);
}

void HashDatabase::delFile(std::string const& filename)
{
	std::string hash, key = static_cast<char>(Key::Filename) + filename;

	leveldb::Status s = m_db->Get(leveldb::ReadOptions(), key, &hash);
	assert(s.ok());
	m_db->Delete(leveldb::WriteOptions(), key);

	key = static_cast<char>(Key::Hash) + hash;
	m_db->Delete(leveldb::WriteOptions(), key);
}

void HashDatabase::delDirectory(std::string path)
{
	assert(path.length() > 0);

	boost::filesystem::path slash("/");
	std::string preferredSlash = slash.make_preferred().string();

	if (path[path.length() - 1] != preferredSlash[0])
		path += preferredSlash[0];

	auto it = m_db->NewIterator(leveldb::ReadOptions());
	std::string begin(1, static_cast<char>(Key::Filename)),
		end(1, static_cast<char>(Key::Partial));

	for (it->Seek(begin), it->Next(); it->Valid() && it->key().ToString() < end; it->Next()) {
		char const* key = it->key().data();
		bool found = true;
		for (int i = 0; i < path.length() && i < it->key().size() - 1; ++i) {
			if (path[i] != key[i + 1]) {
				found = false;
				break;
			}
		}

		if (found && (it->key().size() - 1) >= path.length()) {
			delFile(std::string(it->key().ToString().c_str() + 1, it->key().size() - 1));
		}
	}

	delete it;
}

void HashDatabase::rehash()
{
	auto it = m_db->NewIterator(leveldb::ReadOptions());
	time_t lastWrite;

	std::string begin(1, static_cast<char>(Key::Hash)),
		end(1, static_cast<char>(Key::Filename));

	for (it->Seek(begin), it->Next(); it->Valid() && it->key().ToString() < end; it->Next()) {
		HashDatabase::File::SharedPtr file = unserialize(it->value(), false);
		try {
			lastWrite = boost::filesystem::last_write_time(file->m_filename);

			if (lastWrite != file->m_lastWrite) {
				// Update hash
				addFile(file->m_filename);
			}
		} catch (boost::filesystem::filesystem_error& e) {
			// File no longer exists, remove it from the index
			delFile(file->m_filename);
		}
	}

	delete it;
}

void HashDatabase::list()
{
	auto it = m_db->NewIterator(leveldb::ReadOptions());

	std::string begin(1, static_cast<char>(Key::Hash)),
		end(1, static_cast<char>(Key::Filename));

	for (it->Seek(begin), it->Next(); it->Valid() && it->key().ToString() < end; it->Next()) {
		std::cout << "[";
		for (int i = 1; i <= Hash::SIZE; i += 4) {
			std::cout << std::hex << htonl(*(int*)(it->key().ToString().c_str() + i));
		}
		std::cout << "] " << it->value().ToString().c_str() << std::endl;
	}

	assert(it->status().ok());
	delete it;
}

boost::shared_ptr<std::deque<HashDatabase::File::SharedPtr>> HashDatabase::search(std::string name)
{
	boost::shared_ptr<std::deque<File::SharedPtr>> matches(new std::deque<HashDatabase::File::SharedPtr>());
	auto it = m_db->NewIterator(leveldb::ReadOptions());

	boost::to_lower(name);
	std::string begin(1, static_cast<char>(Key::Filename)),
		end(1, static_cast<char>(Key::Partial));

	for (it->Seek(begin), it->Next(); it->Valid() && it->key().ToString() < end; it->Next()) {
		if (std::string::npos != boost::to_lower_copy(it->key().ToString()).find(name)) {
			File::SharedPtr match = getFile(std::string(it->key().ToString().c_str() + 1, it->key().size() - 1));
			matches->push_back(match);
		}
	}

	assert(it->status().ok());
	delete it;

	return matches;
}

}
