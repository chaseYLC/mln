#pragma once

#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <string.h>

#include "CFCNet_service.h"
#include "container.h"
#include "message_procedure.h"
#include "singleton.h"

namespace CFCNet
{
	enum{
		MAX_NET_ACCEPTOR_COUNT = 10,
		MAX_NET_CONNECTOR_COUNT = 10
	};

	class IEventReceiver;

	class CFCNet_service_impl
		: public CFCNet_service
	{
	public:
		virtual void close(connection::ptr spConn) override;
		virtual boost::asio::io_service& ios() override;
		virtual boost::asio::strand& strand() override;

		inline event_receiver& receiver() { return _receiver;  }
		void initLog();

	private:
		CFCNet_service_impl(boost::asio::io_service& ios, event_receiver& receiver
			, message_procedure::customMessageParser msgParser
			, customMessageManipulator* manip);
		virtual ~CFCNet_service_impl();

		virtual void run() override;
		
		void acceptWait(const std::string& addr, const std::string& port
			, size_t workerThreadsCount
			, const size_t keepAliveMilliseconds_GlobalValue
			) override;

		void accept(const boost::system::error_code& ec);
		void update(const boost::system::error_code& ec);
		void closeSocket(connection::ptr spConn);

		void msgEnque(connection::ptr conn, message::ptr msg);


		friend class connection_impl;

		connection::ptr _acceptConn;

		boost::asio::ip::tcp::acceptor	_acceptor;
		boost::asio::deadline_timer		_updater;
		boost::asio::strand				_strand;
		boost::asio::io_service::work	_work;
		boost::asio::io_service&		_ios;

		boost::chrono::system_clock::time_point _prevTime;

		friend class CFCNet_service;
	};
};//namespace CFCNet
