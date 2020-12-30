#include "stdafx.h"
#include "globalObjects.h"

#include <net/logManager.h>

using namespace mlnserver;

GlobalObjects::GlobalObjects()
{
	m_shared_ios = std::make_shared< boost::asio::io_context  >();

	m_pSystemClockNow = std::move(std::make_unique<TIME_POINT>());
	m_lockSystemClockNow = std::make_unique<mln::Spinlock>();

	m_serviceStop.store(false);

	UpdateTimePoint();
}

GlobalObjects::SHARED_IOC& GlobalObjects::shared_ioc()
{
	return m_shared_ios;
}

std::tuple<uint64_t, GlobalObjects::TIME_POINT> GlobalObjects::UpdateTimePoint() noexcept
{
	std::lock_guard<mln::Spinlock> lock(*m_lockSystemClockNow);

	const uint64_t elapsedMs
		= (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now() - (*m_pSystemClockNow)
			).count();

	*m_pSystemClockNow = std::chrono::system_clock::now();

	return std::make_tuple(elapsedMs, *m_pSystemClockNow);
}