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
	m_shares[dir.name] = dir;
}

void Config::delShare(std::string const& name)
{
	m_shares.erase(name);
}

void Config::load()
{
	try {
		xml_parser::read_xml(m_filename, m_ptree);
	} catch (xml_parser::xml_parser_error& e) {}

	try {
		for (ptree::value_type &v: m_ptree.get_child("config.shares")) {
			if (boost::algorithm::starts_with(v.first, "directory")) {
				m_shares[v.second.data()].name = v.second.data();
				m_shares[v.second.data()].files
					= boost::lexical_cast<unsigned int>(v.second.get_child("<xmlattr>.files").data());
				m_shares[v.second.data()].size
					= boost::lexical_cast<float>(v.second.get_child("<xmlattr>.size").data());
			}
		}
	} catch (ptree_bad_path& e) {}

}

void Config::save()
{
	try {
		m_ptree.get_child("config").erase("shares");
	} catch (ptree_bad_path& e) {}

	int i = 1;
	for (auto const& e: m_shares) {
		std::ostringstream ss;
		ss << "config.shares.directory" << i;
		ptree& node = m_ptree.put(ss.str(), e.second.name);
		node.put("<xmlattr>.files", e.second.files);
		node.put("<xmlattr>.size", e.second.size);
		++i;
	}

	try {
		xml_parser::write_xml(m_filename, m_ptree);
	} catch(xml_parser::xml_parser_error& e) {}
}

}