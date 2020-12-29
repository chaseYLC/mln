#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>
#include <net/macros.h>
#include <chrono>
#include <cstdint>
#include <random>

namespace mln
{
	namespace time
	{
		inline static std::mt19937* s_rand;

		using timeType = int64_t;	// time_t

		template < typename duration >
		timeType currentTimestamp() {
			return std::chrono::duration_cast<duration>(
				std::chrono::system_clock::now().time_since_epoch()
				).count();
		}

		timeType currentTimestampSec(){
			return currentTimestamp<std::chrono::seconds>();
		}

		timeType currentTimestampMs()
		{
			return currentTimestamp<std::chrono::milliseconds>();
		}

		void initRandom() {
			s_rand = new std::mt19937(
				(unsigned int)currentTimestampMs()
			);
		}

		int64_t getRandomNo() {
			assertm(s_rand, "not initialized. call \"initRamdom()\" please.");
			return ((*s_rand)());
		}

		int64_t getRandomNo(const int64_t min, const int64_t max) {
			assertm(s_rand, "not initialized. call \"initRamdom()\" please.");
			std::uniform_int_distribution<int64_t> dist(min, max);
			return dist(*s_rand);
		}

		std::uniform_int_distribution<int64_t> getRandomNoDist(const int64_t min, const int64_t max) {
			assertm(s_rand, "not initialized. call \"initRamdom()\" please.");
			return std::uniform_int_distribution<int64_t>(min, max);
		}


	};//namespace time
}//namespace mln
