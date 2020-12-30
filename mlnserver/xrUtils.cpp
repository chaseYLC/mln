#include "stdafx.h"
#include "xrUtils.h"

// #include <algorithm>
// #include <cctype>
// #include <functional>

namespace DY_Utils {

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

}