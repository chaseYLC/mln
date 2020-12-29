#pragma once

#include <string>
#include <boost/algorithm/string.hpp>

namespace mln 
{
	class StringHelper
	{
	private:
		StringHelper() {}

	public:
		static std::vector<std::string> split(const std::string& src, const std::string& delim)
		{
			std::vector<std::string> out;
			boost::split(out, src, boost::is_any_of(delim));
			return out;
		}
	};

}
