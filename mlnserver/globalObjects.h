#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <tuple>

#include <boost/asio.hpp>
#include <net/singleton.h>
#include <net/lock/spinlock.h>

namespace mln {
	class Spinlock;
}

namespace mlnserver {
	class GlobalObjects
		: public mln::SingletonLight< GlobalObjects >
	{
	public:
		using SHARED_IOS = std::shared_ptr< boost::asio::io_context  >;
		using TIME_POINT = std::chrono::time_point<std::chrono::system_clock>;

		GlobalObjects();

	public:
		SHARED_IOS& shared_ios();

		const TIME_POINT timePointNow() const {
			return *m_pSystemClockNow;
		}

		std::tuple<uint64_t, TIME_POINT> UpdateTimePoint() noexcept;
		void SetServiceStop(const bool stop) { m_serviceStop.store(stop); }
		bool IsServiceStop() const { return m_serviceStop.load(); }
		bool& GetExitFlag() { return m_exitFlag; }


	private:
		SHARED_IOS m_shared_ios;

		std::unique_ptr<TIME_POINT> m_pSystemClockNow;
		std::unique_ptr<mln::Spinlock> m_lockSystemClockNow;
		std::atomic<bool> m_serviceStop;
		bool m_exitFlag = false;
	};
}

