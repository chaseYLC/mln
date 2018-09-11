#include "stdafx.h"
#include "netServiceAcceptor.h"

#include <assert.h>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include "logger.h"
#include <vector>
#include "ConnectionImpl.h"
#include "EventReceiver.h"

namespace MLN
{
	namespace Net
	{
		NetServiceAcceptor::NetServiceAcceptor(ServiceParams &params)
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
				LOGE << ec.message();
			}

			_acceptorSocket.listen(boost::asio::socket_base::max_connections, ec);
			if (ec) {
				LOGE << ec.message();
			}

			auto conn = Connection::ptr(new ConnectionImpl(this, _ios, _msgProc, &_eventReceiver, _keepAliveTimeMs, 0), ConnectionImpl::destruct);
			conn->setServiceID(getIndex());

			_acceptorSocket.async_accept(conn->socket(), _strand.wrap(
				boost::bind(&NetServiceAcceptor::accept_handler, this, boost::asio::placeholders::error, conn)));


			NetService::addWorkerThreads(workerThreadsCount, _ios);

			expireTimerReady();
		}

		void NetServiceAcceptor::accept_handler(const boost::system::error_code& ec, Connection::ptr conn)
		{
			if (!ec)
			{
				_eventReceiver.onAccept(conn);

				conn->start_accept();

				auto newConn = Connection::ptr(new ConnectionImpl(this, _ios, _msgProc, &_eventReceiver, _keepAliveTimeMs, 0), ConnectionImpl::destruct);
				newConn->setServiceID(getIndex());

				_acceptorSocket.async_accept(newConn->socket(), _strand.wrap(
					boost::bind(&NetServiceAcceptor::accept_handler, this, boost::asio::placeholders::error, newConn)));
			}
		}

	};//namespace Net


};//namespace MLN