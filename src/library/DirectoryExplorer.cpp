#include "DirectoryExplorer.hpp"

using namespace std;
namespace fs = boost::filesystem;


namespace Lecista {

DirectoryExplorer::DirectoryExplorer(HashDatabase& db, Config& config)
 : m_db(db), m_config(config)
{

}

template<class T> string DirectoryExplorer::formatSize(T size) {
	string unit;

	if (size < 1000) unit = "B";
	else if ((size /= 1000) < 1000) unit = "kB";
	else if ((size /= 1000) < 1000) unit = "MB";
	else if ((size /= 1000) < 1000) unit = "GB";
	else if ((size /= 1000) < 1000) unit = "TB";
	else { size /= 1000.; unit = "PB"; }

    int precision = 5;
    if (size < 1) precision = 2;
    else if (size < 10) precision = 3;
    else if (size < 100) precision = 4;

    ostringstream ss;
    ss << setprecision(precision) << size;

	return ss.str() + " " + unit;
}

DirectoryExplorer::Directory DirectoryExplorer::addDirectory(string rootpath)
{
	boost::system::error_code ec;
	deque<string> openList, closedList;
	Directory dir;

	try {
		openList.push_back(fs::canonical(rootpath).string());
		dir.name = openList.front();
	} catch (fs::filesystem_error& e) {}

	while (!openList.empty()) {
		string path = openList.front();
		openList.pop_front();
		closedList.push_back(path);

		for (fs::directory_iterator end, i(path, ec); i != end; ++i) {
			try {
				string p = fs::canonical(i->path()).string();
				// Avoid cyclic links
				if (find(closedList.begin(), closedList.end(), p) == closedList.end()) {
					if (fs::is_directory(i->path())) {
						openList.push_back(i->path().string());
					}
					else if (fs::is_regular_file(i->path())) {
						m_db.addFile(i->path().string());
						dir.size += fs::file_size(i->path());
						++dir.files;
					}
				}
			} catch (fs::filesystem_error& e) {}
		}
	}

	m_config.addShare(fs::canonical(rootpath).string());
	return dir;
}

deque<string> DirectoryExplorer::listDirectories()
{
	m_db.list();
	return deque<string>();
}

}