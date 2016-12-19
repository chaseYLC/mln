#include "stdafx.h"
#include "netServiceImpl.h"

#include <assert.h>
#include <Base/Base/include/log.h>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "ConnectionImpl.h"
#include "EventReceiver.h"


namespace MLN
{
	namespace Net
	{
		NetServiceImpl::NetServiceImpl(serviceInitParams &params)
			: NetService(params.receiver, params.msgParser, params.manip)
			, _updater(params.ios, boost::posix_time::milliseconds(params.serviceUpdateTime))
			, _strand(params.ios)
			, _work(params.ios)
			, _ios(params.ios)
			, _updateTimeMs(params.serviceUpdateTime)
		{
		}

		NetServiceImpl::~NetServiceImpl()
		{
		}

		void NetServiceImpl::close(Connection::ptr spConn)
		{
			spConn->set_status(Connection::status::closing);

			_ios.post(
				_strand.wrap(boost::bind(
				&NetServiceImpl::close_handler
				, this
				, spConn)));
		}


		void NetServiceImpl::msgEnque(Connection::ptr spConn, MLN::Base::MessageBuffer::ptr msg)
		{
			spConn->incReadHandlerPendingCount();

			spConn->strand().post(boost::bind(
				&NetServiceImpl::dispatch
				, this
				, spConn
				, msg));
		}

		void NetServiceImpl::expireTimerReady()
		{
			if (0 != _updateTimeMs){
				_prevTime = boost::chrono::system_clock::now();
				_updater.expires_from_now(boost::posix_time::milliseconds(_updateTimeMs));

				_updater.async_wait(_strand.wrap(
					boost::bind(&NetServiceImpl::update_handler, this, boost::asio::placeholders::error)));
			}
		}

		boost::asio::io_service& NetServiceImpl::ios()
		{
			return _ios;
		}

		boost::asio::strand& NetServiceImpl::strand()
		{
			return _strand;
		}

		void NetServiceImpl::close_handler(Connection::ptr spConn)
		{
			boost::weak_ptr<Connection> wp = spConn;
			if (!wp.expired() && wp.lock() != nullptr)
			{
				try {
					boost::system::error_code ec_;
					spConn->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec_);

					if (!ec_ && spConn->get_status() == Connection::status::closing)
					{
						spConn->set_status(Connection::status::close);
						_receiver.onClose(spConn);
					}
					else
					{
						LOGD << "status check : " << ec_.message().data();
					}
				}
				catch (std::exception e) {
					LOGD << "exception in NetServiceAcceptor::closeSocket() => " << e.what();
				}
			}
			else
			{
				LOGD << "NetServiceAcceptor::closeSocket(). already expired";
			}
		}

		void NetServiceImpl::update_handler(const boost::system::error_code& ec)
		{
			if (!ec){
				boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
				unsigned long elapse = (unsigned long)boost::chrono::duration_cast<boost::chrono::milliseconds>(now - _prevTime).count();
				_prevTime = now;

				_receiver.onUpdate(elapse);

				_updater.expires_from_now(boost::posix_time::milliseconds(_updateTimeMs));
				_updater.async_wait(_strand.wrap(
					boost::bind(&NetServiceImpl::update_handler, this, boost::asio::placeholders::error)));
			}
		}

		void NetServiceImpl::sendPacket(void* sendBuffer, const size_t sendSize)
		{
			_conn->sendPacket(sendBuffer, sendSize);
		}

	};//namespace Net

};//namespace MLN