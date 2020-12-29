#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

namespace mln
{
	class Time
	{
	public:
		inline static boost::posix_time::ptime TimeUtil::time_t_epoch(boost::gregorian::date(1970, 1, 1));

		static int64_t getUniversalTimeSec()
		{
			namespace pt = boost::posix_time;

			boost::posix_time::time_duration diff =
				boost::posix_time::second_clock::universal_time() - time_t_epoch;

			return diff.total_seconds();
		}

		static int64_t getUniversalTimeMS()
		{
			namespace pt = boost::posix_time;

			boost::posix_time::time_duration diff =
				boost::posix_time::microsec_clock::universal_time() - time_t_epoch;

			return diff.total_milliseconds();
		}

		static int64_t getLocalTimeSec()
		{
			namespace pt = boost::posix_time;
			pt::ptime now = boost::posix_time::second_clock::local_time();

			static boost::posix_time::ptime time_t_epoch(boost::gregorian::date(1970, 1, 1));
			boost::posix_time::time_duration diff = now - time_t_epoch;

			return diff.total_seconds();
		}

	};
}//namespace mln
