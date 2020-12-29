#pragma once

#include <stdint.h>

namespace DY_Utils {

	extern uint64_t getLocalTimeSec();

	extern std::string MB2UTF8(const char *mbString);

	extern std::string & toLowerCase(std::string &src);

	extern std::string & trim(std::string& s);

	static int getRandomNumber(const int min, const int max);

}
