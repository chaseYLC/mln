#include "stdafx.h"
#include "xrUtils.h"

// #include <algorithm>
// #include <cctype>
// #include <functional>

namespace DY_Utils {

	uint64_t getLocalTimeSec()
	{
		namespace pt = boost::posix_time;
		pt::ptime now = boost::posix_time::second_clock::local_time();

		static boost::posix_time::ptime time_t_epoch(boost::gregorian::date(1970, 1, 1));
		boost::posix_time::time_duration diff = now - time_t_epoch;

		return diff.total_seconds();
	}

	std::string MB2UTF8(const char *mbString)
	{
        #ifdef _WIN32
		wchar_t strUnicode[256] = { 0, };
		int nLen = MultiByteToWideChar(CP_ACP, 0, mbString, (int)strlen(mbString), NULL, NULL);
		MultiByteToWideChar(CP_ACP, 0, mbString, (int)strlen(mbString), strUnicode, nLen);

		char strUtf8[256] = { 0, };
		nLen = WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), strUtf8, nLen, NULL, NULL);

		return strUtf8;
        #else
        return mbString;
        #endif
	}

	std::string & toLowerCase(std::string &src)
	{
		std::transform(src.begin(), src.end(), src.begin(), ::tolower);
		return src;
	}

	std::string & trim(std::string& s)
	{
		// s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		// s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		// return s;

        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c);}));
        return s;
	}

	int getRandomNumber(const int min, const int max)
	{
		std::random_device rn;
		std::mt19937_64 rnd(rn());

		std::uniform_int_distribution<int> range(min, max);

		return range(rnd);
	}

}