#include <iostream>

#include "HashDatabase.hpp"
#include "DirectoryExplorer.hpp"
#include "Config.hpp"

using namespace Lecista;

int main()
{
	Config config;
	HashDatabase db;
	DirectoryExplorer explorer(db, config);

	return 0;
}
