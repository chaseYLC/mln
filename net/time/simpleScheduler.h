#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <stdint.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>

namespace mln
{
	template < size_t MAX_SCHEDULES, size_t TIME_INCREMENT_SIZE_MS, bool USING_EXTERNAL_STRAND >
	class SimpleScheduler
		: public std::enable_shared_from_this< 
			SimpleScheduler<MAX_SCHEDULES, TIME_INCREMENT_SIZE_MS, USING_EXTERNAL_STRAND > >
	{
	public:
		using CallbackType = std::function<void(const uint64_t elapsed)>;
		using ScheduleIDType = size_t;

		struct Info
		{
			ScheduleIDType scheduerID_ = 0;
			const uint64_t interval_;
			int64_t intervalRemain_ = 0;
			CallbackType callback_ = nullptr;

			Info(
				ScheduleIDType scheduerID
				, uint64_t interval
				, CallbackType callback)
				: scheduerID_(scheduerID)
				, interval_(interval)
				, intervalRemain_(interval)
				, callback_(callback)
			{
			}
		};

		using STRAND = boost::asio::io_context::strand;

		SimpleScheduler(boost::asio::io_context &ioc)
			: m_ioc(ioc)
			, m_updateTimer(ioc)
		{
		}

		void Init()
		{
			if (true == USING_EXTERNAL_STRAND) {
				throw std::runtime_error("external starnd");
			}

		}

		void Init(std::shared_ptr<STRAND> spStrand)
		{
			if (false == USING_EXTERNAL_STRAND) {
				throw std::runtime_error("external starnd");
			}

			m_wpStrandExternal = spStrand;
		}

		void TimerStart()
		{
			auto optStrand = GetStrand();

			if (false == optStrand.has_value()) {
				throw std::runtime_error("noen of starnd");
			}
			auto strand = optStrand.value();
			if (!strand) {
				throw std::runtime_error("noen of starnd");
			}
			m_processing = true;
			ReserveUpdate(strand, TIME_INCREMENT_SIZE_MS);
		}

		std::tuple< bool , ScheduleIDType > AddJobInterval(const uint64_t waitCount, CallbackType cb)
		{
			if (0 >= waitCount
				|| nullptr == cb) {
				return std::make_tuple(false, 0);
			}

			if (USING_EXTERNAL_STRAND == false) {
				if (!m_upStrand) {
					m_upStrand = std::make_unique<STRAND>(m_ioc);
				}
			}

			auto optStrand = GetStrand();
			if (false == optStrand.has_value()) {
				throw std::runtime_error("noen of starnd");
			}
			auto strand = optStrand.value();
			if(!strand){
				throw std::runtime_error("noen of starnd");
			}

			const auto entryIndex = m_currentEntries++;

			m_schedulerEntry[entryIndex] = std::make_unique<Info>(
				entryIndex
				, waitCount * TIME_INCREMENT_SIZE_MS
				, cb
				);

			return std::make_tuple(true, entryIndex);
		}

		void TimerStop()
		{
			m_processing = false;

			boost::system::error_code ec;
			m_updateTimer.cancel(ec);
		}

	protected:
		void UpdateHandler(const boost::system::error_code& ec)
		{
			if (USING_EXTERNAL_STRAND) {
				if (!m_wpStrandExternal.lock()) {
					return;
				}
			}

			if (!ec && m_processing) {
				for (auto i = 0; i < m_currentEntries; ++i) {
					auto& ent = *m_schedulerEntry[i].get();

					ent.intervalRemain_ -= TIME_INCREMENT_SIZE_MS;
					if (0 >= ent.intervalRemain_) {
						ent.intervalRemain_ = ent.interval_;
						ent.callback_(0);
					}
				}

				if (GetStrand().has_value()) {
					ReserveUpdate(GetStrand().value(), TIME_INCREMENT_SIZE_MS);
				}
				
			}//if (!ec) {
		}

		std::optional<std::shared_ptr<STRAND>> GetStrand()
		{
			if (USING_EXTERNAL_STRAND) {
				auto spStrand = m_wpStrandExternal.lock();
				if (spStrand) {
					return spStrand;
				}
			}
			else {
				return std::move(m_upStrand);
			}
			return std::nullopt;
		}

		void ReserveUpdate(std::shared_ptr<STRAND> strand, const int interval)
		{
			m_updateTimer.expires_from_now(
				boost::posix_time::milliseconds(interval));

			m_updateTimer.async_wait(strand->wrap(
				boost::bind(&SimpleScheduler::UpdateHandler
					//, shared_from_this()
					, std::enable_shared_from_this<SimpleScheduler<MAX_SCHEDULES, TIME_INCREMENT_SIZE_MS, USING_EXTERNAL_STRAND > >::shared_from_this()
					, boost::asio::placeholders::error)));
		}


	protected:
		boost::asio::deadline_timer m_updateTimer;
		boost::asio::io_context &	m_ioc;
		std::weak_ptr<STRAND> m_wpStrandExternal;
		std::unique_ptr<STRAND> m_upStrand;
		bool m_processing = false;

		std::array< std::unique_ptr<Info>, MAX_SCHEDULES > m_schedulerEntry;
		size_t m_currentEntries = 0;
	};


}//namespace mln