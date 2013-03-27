#pragma once

#include <string>
#include <sstream>
#include <map>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace Lecista {

class Config
{
public:
	struct Directory
	{
		Directory() : size(0), files(0) {}

		std::string name;
		float size;
		unsigned int files;
	};

	Config(std::string const& filename = "config.xml");
	~Config();

	template<class T> T get(std::string const& key) const;
	template<class T> T get(std::string const& key, T def) const;
	template<class T> void set(std::string const& key, T const& value);

	std::map<std::string, Directory> const& shares() const;
	void addShare(Directory const& dir);
	void delShare(std::string const& name);

private:
	std::string m_filename;
	boost::property_tree::ptree m_ptree;
	std::map<std::string, Directory> m_shares;

	void load();
	void save();
};

using namespace boost::property_tree;

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

}