#include "stdafx.h"
#include "netServiceImpl.h"

#include <cassert>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "connectionImpl.h"
#include "eventReceiver.h"
#include "logManager.h"

namespace mln
{
	NetServiceImpl::NetServiceImpl(
		boost::asio::io_context& ios
		, size_t serviceUpdateTimeMs
		, size_t keepAliveTimeMs
		, EventReceiver* eventReceiver
		, MsgUserManip* manip
		, MessageProcedure::customMessageParser msgParser)
		: _updater(ios, boost::posix_time::milliseconds(serviceUpdateTimeMs))
		, _strand(ios)
		, _ios(ios)
		, _updateTimeMs(serviceUpdateTimeMs)
		, _keepAliveTimeMs(keepAliveTimeMs)
		, _manip(manip)
	{
		_msgProc = new MessageProcedure(msgParser, _manip);

		eventReceiver->clone(&_eventReceiver);
		_eventReceiver.initHandler(_msgProc);
	}

	NetServiceImpl::~NetServiceImpl()
	{
		delete _msgProc;
	}

	/*void NetServiceImpl::closeReceiverEvent()
	{
	m_pending = false;

	m_receiverEventClosed = true;

	_strand.post(boost::bind(
	&NetServiceImpl::waitCompletedPending
	, this));

	boost::mutex::scoped_lock lock(m_mutexForPending);
	while (!m_pending) m_conditionForPending.wait(lock);
	}*/

	/*void NetServiceImpl::waitCompletedPending()
	{
	_updater.cancel();

	boost::mutex::scoped_lock lock(m_mutexForPending);
	m_pending = true;
	m_conditionForPending.notify_one();
	}
	*/

	void NetServiceImpl::expireTimerReady()
	{
		if (0 != _updateTimeMs) {
			_prevTime = boost::chrono::system_clock::now();
			_updater.expires_from_now(boost::posix_time::milliseconds(_updateTimeMs));

			_updater.async_wait(boost::asio::bind_executor(_strand, boost::bind(
				&NetServiceImpl::update_handler, this, boost::asio::placeholders::error)));
		}
	}

	boost::asio::io_context& NetServiceImpl::ios()
	{
		return _ios;
	}

	boost::asio::io_context::strand& NetServiceImpl::strand()
	{
		return _strand;
	}

	void NetServiceImpl::update_handler(const boost::system::error_code& ec)
	{
		if (!ec) {
			if (true == isClosedReceiverEvent()) {
				return;
			}

			boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
			unsigned long elapse = (unsigned long)boost::chrono::duration_cast<boost::chrono::milliseconds>(now - _prevTime).count();
			_prevTime = now;

			_eventReceiver.onUpdate(elapse);

			_updater.expires_from_now(boost::posix_time::milliseconds(_updateTimeMs));
			_updater.async_wait(boost::asio::bind_executor(_strand, boost::bind(
				&NetServiceImpl::update_handler, this, boost::asio::placeholders::error)));
		}
	}

};//namespace mln