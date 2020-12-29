#include "stdafx.h"
#include "netServiceAcceptor.h"

#include <assert.h>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include "logManager.h"
#include <vector>
#include "connectionImpl.h"
#include "eventReceiver.h"

namespace mln
{
	NetServiceAcceptor::NetServiceAcceptor(ServiceParams& params)
		: NetServiceImpl(
			params.ios
			, params.serviceUpdateTimeMs
			, params.keepAliveTimeMs
			, &params.receiver
			, params.manip
			, params.msgParser)
		, _acceptorSocket(params.ios)
	{
	}


	NetServiceAcceptor::~NetServiceAcceptor()
	{
		//if (nullptr != _runningAloneObj){
		//	delete _runningAloneObj;
		//}
	}

	void NetServiceAcceptor::acceptWait(const std::string& addr, const std::string& port
		, size_t workerThreadsCount)
	{
		if (0 == workerThreadsCount) {
			workerThreadsCount = boost::thread::hardware_concurrency();
		}

		using TCP = boost::asio::ip::tcp;

		////////////////////////////////////////////////
		// ready accpetor
		TCP::resolver resolver(_ios);

		std::string ipAddr;
		if (true == addr.empty()) {
			ipAddr = "0.0.0.0";
		}
		else {
			ipAddr = addr;
		}

		boost::system::error_code ec;

		TCP::resolver::query quary(ipAddr.c_str(), port);
		TCP::endpoint endpoint = *resolver.resolve(quary, ec);
		_acceptorSocket.open(endpoint.protocol(), ec);

		_acceptorSocket.set_option(TCP::no_delay(true));
		_acceptorSocket.set_option(TCP::acceptor::reuse_address(true));
		_acceptorSocket.set_option(boost::asio::socket_base::linger(true, 0));
		_acceptorSocket.bind(endpoint, ec);

		if (ec) {
			LOGE(ec.message());
		}

		_acceptorSocket.listen(boost::asio::socket_base::max_connections, ec);
		if (ec) {
			LOGE(ec.message());
		}

		auto conn = ConnectionImpl::create(this, _ios, _msgProc, &_eventReceiver, _keepAliveTimeMs, 0);
		conn->setServiceID(getIndex());

		_acceptorSocket.async_accept(conn->socket()
			, boost::asio::bind_executor(_strand, boost::bind(
				&NetServiceAcceptor::accept_handler, this, boost::asio::placeholders::error, conn)));

		NetService::addWorkerThreads(workerThreadsCount, _ios);

		expireTimerReady();
	}

	void NetServiceAcceptor::accept_handler(const boost::system::error_code& ec, Connection::sptr conn)
	{
		if (!ec)
		{
			_eventReceiver.onAccept(conn);

			conn->start_accept();

			auto newConn = ConnectionImpl::create(
				this, _ios, _msgProc, &_eventReceiver, _keepAliveTimeMs, 0);
			newConn->setServiceID(getIndex());

			_acceptorSocket.async_accept(newConn->socket()
				, boost::asio::bind_executor(_strand, boost::bind(
					&NetServiceAcceptor::accept_handler, this, boost::asio::placeholders::error, newConn)));
		}
	}

};//namespace mln