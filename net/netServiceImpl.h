#pragma once

#include <boost/asio.hpp>
#include <boost/chrono.hpp>

#include "singleton.h"
#include "messageProcedure.h"
#include "netService.h"

namespace mln
{
	class EventReceiver;

	class NetServiceImpl
		: public NetService
	{
	public:
		friend class ConnectionImpl;
		friend class NetService;

		boost::asio::io_context& ios() override;
		boost::asio::io_context::strand& strand() override;

		MessageProcedure* getMsgProcedure() const { return _msgProc; }
		bool isClosedReceiverEvent()const { return m_receiverEventClosed; }
		size_t getKeepAliveTimeMs() const { return _keepAliveTimeMs; }

	protected:
		NetServiceImpl(
			boost::asio::io_context& ios
			, size_t serviceUpdateTimeMs
			, size_t keepAliveTimeMs
			, EventReceiver* eventReceiver
			, MsgUserManip* manip
			, MessageProcedure::customMessageParser msgParser
		);

		virtual ~NetServiceImpl();

		void update_handler(const boost::system::error_code& ec);
		void expireTimerReady();

	protected:
		boost::asio::deadline_timer			_updater;
		boost::asio::io_context::strand		_strand;
		boost::asio::io_context& _ios;

		boost::chrono::system_clock::time_point _prevTime;

		size_t _updateTimeMs;
		size_t _keepAliveTimeMs;

		EventReceiver _eventReceiver;
		MsgUserManip* _manip = nullptr;
		MessageProcedure* _msgProc = nullptr;

	private:
		bool m_receiverEventClosed = false;
	};
};//namespace mln
