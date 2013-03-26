#include "DirectoryExplorer.hpp"

namespace Lecista {

namespace fs = boost::filesystem;

DirectoryExplorer::DirectoryExplorer(HashDatabase& db) : m_db(db), m_hashCount(0)
{

}

void DirectoryExplorer::addDirectory(std::string rootpath)
{
	boost::system::error_code ec;
	std::deque<std::string> openList, closedList;

	try {
		openList.push_back(fs::canonical(rootpath).string());
	} catch (fs::filesystem_error& e) {}

	while (!openList.empty()) {
		std::string path = openList.front();
		openList.pop_front();
		closedList.push_back(path);

		for (fs::directory_iterator end, i(path, ec); i != end; ++i) {
			try {
				std::string p = fs::canonical(i->path()).string();
				if (std::find(closedList.begin(), closedList.end(), p) == closedList.end()) {
					if (fs::is_directory(i->path())) {
						openList.push_back(i->path().string());
					}
					else if (fs::is_regular_file(i->path())) {
						m_db.addFile(i->path().string());
						m_hashCount++;
					}
				}
			} catch (fs::filesystem_error& e) {}
		}
	}

}

std::deque<std::string> DirectoryExplorer::listDirectories()
{
	m_db.list();
	return std::deque<std::string>();
}

}