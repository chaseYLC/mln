#pragma once

#include <Base/Base/include/singleton.h>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <string.h>

#include "container.h"
#include "messageProcedure.h"
#include "netService.h"

namespace MLN
{
	namespace Net
	{
		class EventReceiver;

		class NetServiceImpl
			: public NetService
		{
		public:
			friend class ConnectionImpl;
			friend class NetService;

			void close(Connection::ptr spConn) override;
			boost::asio::io_service& ios() override;
			boost::asio::strand& strand() override;
			void sendPacket(void* sendBuffer, const size_t sendSize) override;
			inline EventReceiver& receiver() { return _receiver; }

		protected:
			NetServiceImpl(serviceInitParams &params);
			virtual ~NetServiceImpl();

			void update_handler(const boost::system::error_code& ec);
			void close_handler(Connection::ptr spConn);
			void msgEnque(Connection::ptr conn, MLN::Base::MessageBuffer::ptr msg);
			void expireTimerReady();

		protected:
			Connection::ptr _conn;

			boost::asio::deadline_timer		_updater;
			boost::asio::strand				_strand;
			boost::asio::io_service::work	_work;
			boost::asio::io_service&		_ios;

			boost::chrono::system_clock::time_point _prevTime;

			size_t _updateTimeMs;
		};
	};//namespace Net
};//namespace MLN
