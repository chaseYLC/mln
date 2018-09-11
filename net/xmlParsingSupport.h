#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <tuple>

namespace MLN
{
	namespace Serialization
	{
		class XmlParsingSupport
		{
		public:
			XmlParsingSupport(const std::string &fileName)
			{
				boost::property_tree::xml_parser::read_xml(fileName, _pt_);
			}

			template <typename T>
			void get(T &outValue, const std::string &path)
			{
				outValue = _pt_.get<T>(path);
			}

			template <typename T, typename T2>
			void get(T &outValue, const std::string &path, const T2 &defaultValue)
			{
				outValue = _pt_.get<T>(path, defaultValue);
			}

			boost::property_tree::ptree & getPtree() {
				return _pt_;
			}

		private:
			boost::property_tree::ptree _pt_;
		};
	};// namespace Serialization
};


#define XML_PARSING_BEGIN(fileName)	try{MLN::Serialization::XmlParsingSupport _xml_parser_(fileName);

#define XML_PARSING_END		}catch (boost::property_tree::xml_parser::xml_parser_error &e){	LOGE << e.message();return false;}\
	catch (boost::property_tree::ptree_error &e){ LOGE << e.what();	return false; }

#define XML_PARSING_GET	_xml_parser_.get

#define XML_TREEOBJ_REF	_xml_parser_.getPtree()