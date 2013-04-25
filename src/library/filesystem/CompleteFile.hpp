#pragma once

#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include "File.hpp"

namespace Lecista {

class CompleteFile : public File
{
public:
	CompleteFile(std::string const& filename);

private:
	void hash(std::string const& filename);
};

}
