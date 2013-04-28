#include "File.hpp"

namespace Lecista {

File::File(uintmax_t filesize, Hash::SharedPtr const& rootHash)
 : m_filesize(filesize),
   m_blocks(std::max(1, (int)ceil(m_filesize / (float)BLOCK_SIZE)), false)
{
	initTree(rootHash);
}

File::File(char const* data, unsigned int size)
{
	unsigned int index = sizeof m_filesize;
	Hash nullHash;

	std::memcpy(&m_filesize, data, index);
	m_blocks = std::vector<bool>(
		std::max(1, (int)ceil(m_filesize / (float)BLOCK_SIZE)), false);

	index += unpackStates(data + index);
	initTree(Hash::SharedPtr());
	for (auto &level: m_tree) {
		for (auto &hash: level) {
			if (index + Hash::SIZE > size) {
				throw UnserializeException();
			}

			hash = Hash::SharedPtr(new Hash(data + index));
			if (nullHash == *hash) {
				hash = Hash::SharedPtr();
			}

			index += Hash::SIZE;
		}
	}
}

File::File(std::string const& serialized) : File(serialized.c_str(), serialized.size())
{

}

File::~File()
{

}

void File::initTree(Hash::SharedPtr const& rootHash)
{
	m_tree.clear();
	for (int i = m_blocks.size(); i > 1 ; i = ceil(i / 2.)) {
		m_tree.push_back(std::vector<Hash::SharedPtr>(i));
	}

	m_tree.push_back(std::vector<Hash::SharedPtr>(1));
	m_tree.back()[0] = rootHash;
}

void File::setHash(unsigned int block, Hash::SharedPtr const& hash)
{
	Hasher hasher;
	m_tree.front()[block] = hash;

	for (int levelNum = 0; levelNum < m_tree.size() - 1; ++levelNum, block /= 2) {
		unsigned int sibling = block + ((block % 2 == 0) ? 1 : -1);
		std::vector<Hash::SharedPtr>& level = m_tree[levelNum];
		Hash::SharedPtr h(m_tree[levelNum][block]);

		if (h && sibling >= level.size()) {
			m_tree[levelNum + 1][block / 2] = h;
		}
		else if (h && level[sibling]) {
			hasher.reset();

			if (sibling > block) {
				hasher.update(h);
				hasher.update(level[sibling]);
			}
			else {
				hasher.update(level[sibling]);
				hasher.update(h);
			}

			m_tree[levelNum + 1][block / 2] = hasher.getHash();
		}
	}
}

std::vector<Hash::SharedPtr> File::hashList(unsigned int block) const
{
	std::vector<Hash::SharedPtr> list(m_tree.size() - 1);
	unsigned int parent = block, sibling;

	if (m_tree.size() < 2) {
		// Nothing to do, file is contained in one block
	}
	else {
		// Get hashes of sibling and parents' siblings
		unsigned int index = 0;
		for (auto i = m_tree.begin(); i + 1 != m_tree.end(); ++i, parent /= 2) {
			sibling = parent + ((parent % 2 == 0) ? 1 : -1);

			if (sibling < i->size()) {
				list[index++] = (*i)[sibling];
			}
			else { // Null hash for orphan branches
				list[index++] = Hash::SharedPtr(new Hash());
			}
		}
	}

	return list;
}

bool File::checkBlock(
	unsigned int block,
	Hash::SharedPtr const& hash,
	std::vector<Hash::SharedPtr> const& hashList)
{
	Hasher hasher;
	int parent = block;
	Hash::SharedPtr globalHash = hash;

	if (hashList.size() != m_tree.size()) {
		return false;
	}

	for (Hash::SharedPtr const& h: hashList) {
		hasher.reset();

		if (parent % 2 == 0) {
			hasher.update(globalHash);
			hasher.update(h);
		}
		else {
			hasher.update(h);
			hasher.update(globalHash);
		}

		globalHash = hasher.getHash();
		parent /= 2;
	}

	bool ok = (*globalHash == *rootHash());
	if (ok) {
		setHash(block, hash);
		m_blocks[block] = true;

		for (unsigned int level = 1, sibling = block / 2;
			 level < hashList.size();
			 ++level, sibling /= 2)
		{
			sibling += (sibling % 2 == 0) ? 1 : -1;
			m_tree[level][sibling] = hashList[level];
		}

		if (hashList.size() > 0) { // Only for non-monoblock files
			setHash(block + ((block % 2 == 0) ? 1 : -1), hashList[0]);
		}
	}

	return ok;
}

bool File::isComplete() const
{
	for (bool s: m_blocks) {
		if (!s) {
			return false;
		}
	}

	return true;
}

unsigned int File::serializedSize() const
{
	// How many hashes are contained in this tree ?
	unsigned int numHashes = 0;
	for (std::vector<Hash::SharedPtr> const& level: m_tree) {
		numHashes += level.size();
	}

	return sizeof(m_filesize)
		+ ceil(m_blocks.size() / 8.)
		+ numHashes * Hash::SIZE;
}

unsigned int File::serialize(char*& out) const
{
	unsigned int i = 0;
	if (out == 0) {
		out = new char[serializedSize()];
	}

	std::memcpy(out + i, &m_filesize, sizeof m_filesize);
	i += sizeof m_filesize;

	i += packStates(out + i);

	for (auto j = m_tree.begin(); j != m_tree.end(); ++j) {
		for (auto k = j->begin(); k != j->end(); ++k, i += Hash::SIZE) {
			if (*k) {
				std::memcpy(out + i, (*k)->data(), Hash::SIZE);
			}
			else {
				std::memset(out + i, 0, Hash::SIZE);
			}
		}
	}

	return i;
}

unsigned int File::packStates(char* out) const
{
	unsigned int bytes = std::max(1, (int)ceil(m_blocks.size() / 8.));
	std::memset(out, 0, bytes);

	for (int i = 0; i < m_blocks.size(); ++i) {
		if (m_blocks[i]) {
			out[i / 8] |= (1 << (i % 8));
		}
	}

	return bytes;
}

unsigned int File::unpackStates(char const* out)
{
	for (int i = 0; i < m_blocks.size(); ++i) {
		m_blocks[i] = out[i / 8] & (1 << (i % 8));
	}

	return std::max(1, (int)ceil(m_blocks.size() / 8.));
}

void File::print() const
{
	std::cout << m_filesize << " b, "
		<< m_blocks.size() << " blocks " << rootHash()->string() << std::endl;

	int i = 0, j = 0;
	for (auto level: m_tree) {
		std::cout << "[" << i << "] ";

		int w = ((1 << i) - 1)*11;
		++i;

		for (auto node = level.begin(); node != level.end(); ++node) {
			std::cout << std::setw(w / 2) << "";
			if (*node) {
				std::cout << (*node)->string().substr(0, 8);
			}
			else {
				std::cout << "........";
			}

			std::cout << std::setw(ceil(w / 2.)) << "";
			if (node + 1 != level.end()) {
				if (level == m_tree.front() && m_blocks[j++]) {
					std::cout << "*| ";
				}
				else {
					std::cout << " | ";
				}
			}
			else if (level == m_tree.front() && m_blocks[j++]) {
				std::cout << "*";
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

}
