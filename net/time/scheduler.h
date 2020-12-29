#pragma once

#include <functional>
#include <memory>
#include <stdint.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>

namespace mln
{
	template < size_t MAX_SCHEDULES >
	class Scheduler
	{
	public:
		using CallbackType = std::function<void(const uint64_t elapsed)>;
		using ScheduleIDType = size_t;

		struct Info
		{
			ScheduleIDType scheduerID_ = 0;
			uint64_t updateTimeMs_ = 0;
			boost::chrono::system_clock::time_point updatePrevTime_;
			boost::asio::deadline_timer updateTimer_;
			CallbackType callback_ = nullptr;

			Info(
				ScheduleIDType scheduerID
				, uint64_t updateTimeMs
				, CallbackType callback
				, boost::asio::io_context& ios)
				: scheduerID_(scheduerID)
				, updateTimeMs_(updateTimeMs)
				, callback_(callback)
				, updateTimer_(ios)
			{
			}
		};

		Scheduler(boost::asio::io_context & ioc)
			: m_ioc(ioc)
			, m_strand(ioc)
		{
		}

		std::tuple< bool , ScheduleIDType > AddJobInterval(const uint64_t updateTimeMs, CallbackType cb)
		{
			if (0 >= updateTimeMs
				|| nullptr == cb) {
				return std::make_tuple(false, 0);
			}

			if (MAX_SCHEDULES <= m_currentEntries) {
				throw std::runtime_error("m_schedulerEntry => buffer overflow");
			}

			const auto entryIndex = m_currentEntries++;

			m_schedulerEntry[entryIndex] = std::make_unique<Scheduler::Info>(entryIndex
				, updateTimeMs
				, cb
				, m_ioc
				);

			auto& ent = *m_schedulerEntry[entryIndex];

			ent.updatePrevTime_ = boost::chrono::system_clock::now();
			ent.updateTimer_.expires_from_now(boost::posix_time::milliseconds(ent.updateTimeMs_));

			ent.updateTimer_.async_wait(boost::asio::bind_executor(m_strand,
				boost::bind(&Scheduler::UpdateHandler, this, ent.scheduerID_, boost::asio::placeholders::error)));

			return std::make_tuple(true, entryIndex);
		}

	protected:
		void UpdateHandler(const ScheduleIDType schedulerID, const boost::system::error_code& ec)
		{
			if (!ec) {
				auto& ent = *m_schedulerEntry[schedulerID].get();

				boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
				const unsigned long elapsed
					= (unsigned long)boost::chrono::duration_cast<boost::chrono::milliseconds>(now - ent.updatePrevTime_).count();
				ent.updatePrevTime_ = now;

				// call registed timer.
				ent.callback_(elapsed);

				ent.updateTimer_.expires_from_now(boost::posix_time::milliseconds(ent.updateTimeMs_));
				ent.updateTimer_.async_wait(boost::asio::bind_executor(m_strand,
					boost::bind(&Scheduler::UpdateHandler, this, ent.scheduerID_, boost::asio::placeholders::error)));
			}//if (!ec) {
		}

	protected:
		boost::asio::io_context &m_ioc;
		boost::asio::io_context::strand m_strand;

		std::array< std::unique_ptr<Info>, MAX_SCHEDULES > m_schedulerEntry;
		size_t m_currentEntries = 0;
	};


}//namespace mln