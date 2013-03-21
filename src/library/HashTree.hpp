#pragma once

#include <cmath>
#include <string>
#include <fstream>
#include <vector>
#include <deque>
#include "Hasher.hpp"

namespace Lecista {
	
class HashTree
{
public:
	HashTree(std::string filename);
	std::deque<std::vector<Hash::SharedPtr> > hash();
	static std::deque<std::vector<Hash::SharedPtr> > hashFile(std::string filename);

private:
	static const int BLOCK_SIZE = 16; // 16 Mo
	
	std::string m_filename;
	std::deque<std::vector<Hash::SharedPtr> > tree;
	
	bool hashBase();
	unsigned int hashLevel();
};

}