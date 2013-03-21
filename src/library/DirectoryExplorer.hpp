#pragma once

#include <string>
#include <deque>

namespace Lecista {

class DirectoryExplorer
{
public:
	struct Directory
	{
		std::string name;
	};

	DirectoryExplorer();
	std::deque<std::string> getStructure(std::string path);

protected:
	//std::vector<
};

}