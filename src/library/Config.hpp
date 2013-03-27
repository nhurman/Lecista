#pragma once

#include <string>
#include <sstream>
#include <set>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace Lecista {

class Config
{
public:
	inline Config(std::string const& filename = "config.xml");
	inline ~Config();

	template<class T> T get(std::string const& key) const;
	template<class T> T get(std::string const& key, T def) const;
	template<class T> void set(std::string const& key, T const& value);

	inline std::set<std::string> const& shares() const;
	inline void addShare(std::string const& name);
	inline void delShare(std::string const& name);

private:
	std::string m_filename;
	boost::property_tree::ptree m_ptree;
	std::set<std::string> m_shares;

	inline void load();
	inline void save();
};


using namespace boost::property_tree;

Config::Config(std::string const& filename) : m_filename(filename)
{
	load();
}

Config::~Config()
{
	save();
}

template<class T> T Config::get(std::string const& key, T def) const
{
	return m_ptree.get<T>("config." + key, def);
}

template<class T> T Config::get(std::string const& key) const
{
	return m_ptree.get<T>("config." + key);
}

template<class T> void Config::set(std::string const& key, T const& value)
{
	m_ptree.put<T>("config." + key, value);
}

std::set<std::string> const& Config::shares() const
{
	return m_shares;
}

void Config::addShare(std::string const& name)
{
	m_shares.insert(name);
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
				m_shares.insert(v.second.data());
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
	for (std::string const& name: m_shares) {
		std::ostringstream ss;
		ss << "config.shares.directory" << i;
		m_ptree.put(ss.str(), name);
		++i;
	}

	try {
		xml_parser::write_xml(m_filename, m_ptree);
	} catch(xml_parser::xml_parser_error& e) {}
}

}