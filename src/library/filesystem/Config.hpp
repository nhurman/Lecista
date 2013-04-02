#pragma once

#include <string>
#include <sstream>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "../Logger.hpp"

namespace Lecista {

//! Stores and loads configuration option using an xml file.
/*!
  This class is used to store configuration options like user preferences
  and shared directories.
*/
class Config
{
public:
	//! Represents a top-level shared directory.
	struct Directory
	{
		Directory() : size(0), files(0) {}

		std::string path;
		float size;
		unsigned int files;
	};

	/*!
	  \param filename Path of the file where the configuration is saved
	*/
	Config(std::string const& filename = "config.xml");

	~Config();

	//! Fetch a configuration option by key.
	/*!
	  \param key Option unique identifier
	  \throws boost::property_tree::ptree_bad_path Key does not exist
	  \return Fetched value
	*/
	template<class T> T get(std::string const& key) const;

	//! Fetch a configuration option by key, with a default value.
	/*! Use a default value instead of throwing an exception when the key does not exist.
	  \param key Option unique identifier
	  \param def Default value
	  \return Fetched value, or default value if the key does not exist
	*/
	template<class T> T get(std::string const& key, T def) const;

	//! Set a configuration option.
	/*!
	  \param key Option unique identifier
	  \param value
	*/
	template<class T> void set(std::string const& key, T const& value);

	//! Fetche the list of shared directories.
	/*!
	  \return Shares list, where the key is the directory name
	*/
	std::map<std::string, Directory> const& shares() const;

	//! Add a shared directory to the configuration.
	/*! This method does not hash the directory content.
	  \param dir
	  \sa Lecista::DirectoryExplorer::addDirectory
	*/
	void addShare(Directory const& dir);

	//! Remove a shared directory from the configuration.
	/*! This method does not remove the directory content from the index.
	  \param path Directory path (path attribute of the Directory structure)
	  \sa DirectoryExplorer::delDirectory
	*/
	void delShare(std::string const& path);

private:
	std::string m_filename; /*!< XML file used to save and load the configuration */
	boost::property_tree::ptree m_ptree; /*!< In-memory representation  of the configuration */
	std::map<std::string, Directory> m_shares; /*!< List of shared directories */

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
