#include "stdafx.h"
#include "netServiceAcceptor.h"

#include <assert.h>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <Base/Base/include/logService.h>
#include <Base/Base/include/runningAlone.h>


#include <vector>

#include "ConnectionImpl.h"
#include "EventReceiver.h"

namespace MLN
{
	namespace Net
	{
		NetServiceAcceptor::NetServiceAcceptor(serviceInitParams &params)
			: NetServiceImpl(params)
			, _acceptor(params.ios)
		{
		}


		NetServiceAcceptor::~NetServiceAcceptor()
		{
			if (nullptr != _runningAloneObj){
				delete _runningAloneObj;
			}
		}

		void NetServiceAcceptor::acceptWait(const std::string& addr, const std::string& port
			, size_t workerThreadsCount
			, const size_t keepAliveMilliseconds)
		{
			if (0 == workerThreadsCount){
				workerThreadsCount = boost::thread::hardware_concurrency();
			}

			////////////////////////////////////////////////
			// set keepAliveTime
			_keepAliveTimeMs = keepAliveMilliseconds;


			using TCP = boost::asio::ip::tcp;

			////////////////////////////////////////////////
			// ready accpetor
			TCP::resolver resolver(_ios);

			std::string ipAddr;
			if (true == addr.empty()){
				ipAddr = "0.0.0.0";
			}
			else{
				ipAddr = addr;
			}

			boost::system::error_code ec;

			TCP::resolver::query quary(ipAddr.c_str(), port);
			TCP::endpoint endpoint = *resolver.resolve(quary, ec);
			_acceptor.open(endpoint.protocol(), ec);

			_acceptor.set_option(TCP::no_delay(true));
			_acceptor.set_option(TCP::acceptor::reuse_address(true));
			_acceptor.bind(endpoint, ec);

			if (ec){
				LOGE << ec.message();
			}

			_acceptor.listen(boost::asio::socket_base::max_connections, ec);
			if (ec){
				LOGE << ec.message();
			}

			_conn = Connection::ptr(new ConnectionImpl(this, _ios, _packetHeaderManip), ConnectionImpl::destruct);
			_acceptor.async_accept(_conn->socket(), _strand.wrap(
				boost::bind(&NetServiceAcceptor::accept_handler, this, boost::asio::placeholders::error)));


			NetService::addWorkerThreads(workerThreadsCount, _ios);

			expireTimerReady();
		}

		void NetServiceAcceptor::connectWait(const std::string& addr, const std::string& port)
		{
			assert(false && "acceptor can't connect");
		}

		void NetServiceAcceptor::accept_handler(const boost::system::error_code& ec)
		{
			if (!ec)
			{
				_conn->start_accept();

				_receiver.onAccept(_conn);

				_conn = Connection::ptr(new ConnectionImpl(this, _ios, _packetHeaderManip), ConnectionImpl::destruct);
				_acceptor.async_accept(_conn->socket(), _strand.wrap(
					boost::bind(&NetServiceAcceptor::accept_handler, this, boost::asio::placeholders::error)));
			}
		}

		size_t NetServiceAcceptor::getKeepAliveTimeMs() const
		{
			return _keepAliveTimeMs;
		}

		MLN::Base::RunningAlone * NetServiceAcceptor::getRunningAloneCheckObject()
		{
			if (nullptr == _runningAloneObj){
				_runningAloneObj = new MLN::Base::RunningAlone();
			}
			return _runningAloneObj;
		}

	};//namespace Net


};//namespace MLN