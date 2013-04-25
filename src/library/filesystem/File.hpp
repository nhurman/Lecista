#pragma once

#include <deque>
#include <exception>
#include <vector>
#include <string>

#include <boost/asio.hpp>

#include "Hash.hpp"
#include "Hasher.hpp"
#include "../Logger.hpp"

namespace Lecista {

class File
{
public:
	class UnserializeException : public std::exception {};
	static unsigned int const BLOCK_SIZE = 16 * 1000 * 1000; // 16 MB

	File(uintmax_t filesize, Hash::SharedPtr const& rootHash);
	File(char const* data, unsigned int size);
	File(std::string const& serialized);
	~File();

	uintmax_t filesize() const { return m_filesize; }
	Hash::SharedPtr rootHash() const { return m_tree.back().front(); }

	void setHash(unsigned int block, Hash::SharedPtr const& hash);
	std::vector<Hash::SharedPtr> hashList(unsigned int block) const;
	bool checkBlock(
		unsigned int block,
		Hash::SharedPtr const& hash,
		std::vector<Hash::SharedPtr> const& hashList);
	bool isComplete() const;

	unsigned int serializedSize() const;
	unsigned int serialize(char*& out) const;

	void print() const;

protected:
	uintmax_t m_filesize;

	std::deque<std::vector<Hash::SharedPtr>> m_tree;
	std::vector<bool> m_blocks;

	void initTree(Hash::SharedPtr const& rootHash);

private:
	unsigned int packStates(char* out) const;
	unsigned int unpackStates(char const* out);
};

}
