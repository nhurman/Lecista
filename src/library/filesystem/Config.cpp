#include "Config.hpp"

namespace Lecista {

Config::Config(std::string const& filename) : m_filename(filename)
{
	load();
}

Config::~Config()
{
	save();
}

std::map<std::string, Config::Directory> const& Config::shares() const
{
	return m_shares;
}

void Config::addShare(Config::Directory const& dir)
{
	m_shares[dir.path] = dir;
}

void Config::delShare(std::string const& path)
{
	m_shares.erase(path);
}

void Config::load()
{
	try {
		xml_parser::read_xml(m_filename, m_ptree);
	} catch (xml_parser::xml_parser_error&) {}

	try {
		for (ptree::value_type &v: m_ptree.get_child("config.shares")) {
			if (boost::algorithm::starts_with(v.first, "directory")) {
				std::string key = v.second.data();
				m_shares[key].path = v.second.data();
				m_shares[key].files
					= boost::lexical_cast<unsigned int>(v.second.get_child("<xmlattr>.files").data());
				m_shares[key].size
					= boost::lexical_cast<float>(v.second.get_child("<xmlattr>.size").data());
			}
		}
	} catch (ptree_bad_path&) {}

}

void Config::save()
{
	try {
		m_ptree.get_child("config").erase("shares");
	} catch (ptree_bad_path&) {}

	int i = 1;
	for (auto const& e: m_shares) {
		std::ostringstream ss;
		ss << "config.shares.directory" << i;
		ptree& node = m_ptree.put(ss.str(), e.second.path);
		node.put("<xmlattr>.files", e.second.files);
		node.put("<xmlattr>.size", e.second.size);
		++i;
	}

	try {
		xml_parser::write_xml(m_filename, m_ptree);
	} catch(xml_parser::xml_parser_error&) {}
}

}
