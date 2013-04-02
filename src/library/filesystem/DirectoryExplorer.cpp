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
	else { size /= 1000 ; unit = "PB"; }

	int precision = 5;
	if (size < 1) precision = 2;
	else if (size < 10) precision = 3;
	else if (size < 100) precision = 4;

	ostringstream ss;
	ss << setprecision(precision) << size;

	return ss.str() + " " + unit;
}

Config::Directory DirectoryExplorer::addDirectory(string rootpath, bool rescan)
{
	boost::system::error_code ec;
	deque<string> openList, closedList;
	Config::Directory dir;

	try {
		rootpath = fs::canonical(rootpath).string();
		openList.push_back(rootpath);
	} catch (fs::filesystem_error& e) {
		return dir;
	}

	boost::filesystem::path slash("/");
	std::string preferredSlash = slash.make_preferred().native();

	if (rootpath[rootpath.length() - 1] != preferredSlash[0])
		rootpath += preferredSlash[0];

	dir.path = rootpath;

	if (!rescan) { // Check if this directory is already shared
		for (auto e: m_config.shares()) {
			bool found = true;
			for (int i = 0; i < rootpath.length() && i < e.second.path.length(); ++i) {
				if (rootpath[i] != e.second.path[i]) {
					found = false;
					break;
				}
			}

			if (found && rootpath.length() >= e.second.path.length()) {
				// Only perform a scan of added/removed files and update index
				rescan = true;
			}
		}
	}

	while (!openList.empty()) {
		string path = openList.front();
		openList.pop_front();
		closedList.push_back(path);

		for (fs::directory_iterator end, i(path, ec); i != end; ++i) {
			try {
				string c = fs::canonical(i->path()).string();
				string p = i->path().string();

				// Avoid cyclic links
				if (find(closedList.begin(), closedList.end(), c) == closedList.end()) {
					if (fs::is_directory(i->path())) {
						openList.push_back(p);
					}
					else if (fs::is_regular_file(i->path())) {
						if (!rescan || !m_db.exists(p)) {
							m_db.addFile(p);
						}

						dir.size += fs::file_size(i->path());
						++dir.files;
					}
				}
			} catch (fs::filesystem_error& e) {}
		}
	}

	m_config.addShare(dir);
	return dir;
}

void DirectoryExplorer::delDirectory(string rootpath)
{
	rootpath = fs::canonical(rootpath).string();
	deque<string> toDelete;

	for (auto const& e: m_config.shares()) {
		bool found = true;
		for (int i = 0; i < rootpath.length() && i < e.second.path.length(); ++i) {
			if (rootpath[i] != e.second.path[i]) {
				found = false;
				break;
			}
		}

		if (found && rootpath.length() <= e.second.path.length()) {
			toDelete.push_back(e.second.path) ;
		}
	}

	for (string const& s: toDelete) {
		m_config.delShare(s);
		m_db.delDirectory(s);
	}
}

map<string, Config::Directory> const& DirectoryExplorer::listDirectories() const
{
	map<string, Config::Directory> const& shares = m_config.shares();

	for (auto const& e: shares) {
		LOG_DEBUG(e.second.path << " (" << e.second.files << " files, "
			<< formatSize(e.second.size) << ")");
	}

	return shares;
}

void DirectoryExplorer::rescan()
{
	for (auto const& e: m_config.shares()) {
		addDirectory(e.second.path, true);
	}
}

}
