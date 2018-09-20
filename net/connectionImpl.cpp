#include "stdafx.h"
#include "ConnectionImpl.h"

#include <atomic>
#include <boost/bind.hpp>
#include <boost/asio/write.hpp>
#include "messageBuffer.h"
#include "netServiceAcceptor.h"
#include "logger.h"

namespace MLN
{
	namespace Net
	{
		std::atomic< size_t > s_identitySeed = { 1 };

		Connection::~Connection()
		{
		}

		ConnectionImpl::ConnectionImpl(NetService* owner, boost::asio::io_service& ios
			, MessageProcedure *msgProc, EventReceiver *evntReceiver
			, const size_t keepAliveTimeMs
			, const size_t connectionID
			)
			: _ios(ios)
			, _socket(ios)
			, _strand(ios)
			, _status(status::close)
			, _tag(nullptr)
			, _msgProc(msgProc)
			, _msgManipulator(msgProc->_packetHeaderManip)
			, _keepTimer(ios)
			, _closeReserveTimer(ios)
			, _postRetryCount(0)
			, _eventReceiver(evntReceiver)
			, _keepAliveTime(keepAliveTimeMs)
			, _connectionID(connectionID)
		{
			memset(_syncEncryptionKey, 0, sizeof(_syncEncryptionKey));

			_identity = s_identitySeed.fetch_add(1, std::memory_order_relaxed);

			renewExpireTime();
		}

		void ConnectionImpl::set_status(const status s)
		{
			_status = s;
		}

		Connection::status ConnectionImpl::get_status() const
		{
			return _status;
		}

		boost::asio::ip::tcp::socket& ConnectionImpl::socket()
		{
			return _socket;
		}

		boost::asio::strand& ConnectionImpl::strand()
		{
			return _strand;
		}

		void ConnectionImpl::setTag(void* tag)
		{
			_tag = tag;
		}

		void* ConnectionImpl::getTag() const
		{
			return _tag;
		}

		size_t ConnectionImpl::getIdentity() const
		{
			return _identity;
		}

		void ConnectionImpl::setServiceID(const size_t id)
		{
			_netServiceID = id;
		}

		size_t ConnectionImpl::getServiceID() const
		{
			return _netServiceID;
		}

		size_t ConnectionImpl::getConnectionID() const
		{
			return _connectionID;
		}

		bool ConnectionImpl::isSyncEncryptionKey(const uint8_t encType) const
		{
			return _syncEncryptionKey[encType];
		}

		void ConnectionImpl::setSyncEncryptionKey(const uint8_t encType, const bool sync)
		{
			_syncEncryptionKey[encType] = sync;
		}

		void ConnectionImpl::start_accept()
		{
			_msg = MessageBuffer::Ptr(new MessageBuffer(_msgManipulator), MessageBuffer::destruct);

			_socket.set_option(boost::asio::ip::tcp::no_delay(true));
			_socket.set_option(boost::asio::socket_base::linger(true, 0));

			_socket.async_read_some(
				boost::asio::buffer(_msg->enableBuffer(), _msg->remainWriteSize())
				, _strand.wrap(boost::bind(&ConnectionImpl::read_handler
					, shared_from_this()
					, boost::asio::placeholders::error
					, boost::asio::placeholders::bytes_transferred)));

			_status = status::open;

			renewExpireTime();
		}

		void ConnectionImpl::start_connect()
		{
			_msg = MessageBuffer::Ptr(new MessageBuffer(_msgManipulator), MessageBuffer::destruct);

			_socket.set_option(boost::asio::ip::tcp::no_delay(true));
			_socket.set_option(boost::asio::socket_base::linger(true, 0));

			_socket.async_read_some(
				boost::asio::buffer(_msg->enableBuffer(), _msg->remainWriteSize())
				, _strand.wrap(boost::bind(&ConnectionImpl::read_handler
					, shared_from_this()
					, boost::asio::placeholders::error
					, boost::asio::placeholders::bytes_transferred)));

			_status = status::open;

			renewExpireTime();
		}

		void ConnectionImpl::send(MessageBuffer& msg)
		{
			msg.facilitate();

			boost::asio::async_write(
				_socket
				, boost::asio::buffer(msg.data(), msg.size())
				, _strand.wrap(boost::bind(&ConnectionImpl::write_handler
					, shared_from_this()
					, boost::asio::placeholders::error
					, boost::asio::placeholders::bytes_transferred))
				);
		}

		void ConnectionImpl::sendRaw(void* sendBuffer, const size_t sendSize)
		{
			boost::asio::async_write(
				_socket
				, boost::asio::buffer(sendBuffer, sendSize)
				, _strand.wrap(boost::bind(&ConnectionImpl::write_handler
					, shared_from_this()
					, boost::asio::placeholders::error
					, boost::asio::placeholders::bytes_transferred))
				);
		}

		void ConnectionImpl::sendPacket(void* sendBuffer, const size_t sendSize)
		{
			MessageBuffer msg(_msgManipulator, true);
			msg.write((char*)sendBuffer, sendSize);
			this->send(msg);
		}

		void ConnectionImpl::read_handler(const boost::system::error_code& ec, size_t bytes_transferred)
		{
			if (!ec)
			{
				_msg->write(bytes_transferred);

				MessageBuffer::Ptr msgRecv = _msg;
				_msg = MessageBuffer::Ptr(new MessageBuffer(_msgManipulator), MessageBuffer::destruct);

				incReadHandlerPendingCount();

				_strand.post(boost::bind(
					&MessageProcedure::dispatch
					, _msgProc
					, shared_from_this()
					, msgRecv));

				_socket.async_read_some(
					boost::asio::buffer(_msg->enableBuffer(), _msg->remainWriteSize())
					, _strand.wrap(boost::bind(&ConnectionImpl::read_handler
						, shared_from_this()
						, boost::asio::placeholders::error
						, boost::asio::placeholders::bytes_transferred)));

				renewExpireTime();
			}
			else
			{
				if (status_codes::SHUTDOWN != ec.value()	// shutdown
					&& boost::asio::error::eof != ec.value()	// close by client
					&& 0 < _readHandlerPending)
				{
					if (5 > _postRetryCount++) {

						_strand.post(boost::bind(
							&ConnectionImpl::read_handler
							, shared_from_this()
							, ec
							, bytes_transferred));

						return;
					}

					LOGW << "increase postRetryCount Limit. msg value : " << std::to_string(ec.value())
						<< ", string : " << ec.message().data()
						<< ", pending : " << std::to_string(_readHandlerPending);
				}

				closing(boost::asio::ip::tcp::socket::shutdown_receive);
			}
		}


		void ConnectionImpl::closing(boost::asio::socket_base::shutdown_type what)
		{
			boost::system::error_code ec;
			_socket.shutdown(what, ec);

			if (!ec && _status == status::open)
			{
				_status = status::closing;

				_keepTimer.cancel();
				_closeReserveTimer.cancel();

				_ios.post(
					_strand.wrap(boost::bind(
						&ConnectionImpl::close
						, shared_from_this())));
			}
		}


		void ConnectionImpl::write_handler(const boost::system::error_code& ec, size_t bytes_transferred)
		{
			if (!ec) {
				renewExpireTime();
				return;
			}

			closing(boost::asio::ip::tcp::socket::shutdown_send);
		}


		void ConnectionImpl::renewExpireTime()
		{
			if (0 == _keepAliveTime) {
				return;
			}

			_keepTimer.expires_from_now(
				boost::posix_time::milliseconds(_keepAliveTime));

			_keepTimer.async_wait(_strand.wrap(
				boost::bind(&ConnectionImpl::onExpireTime
					, this
					, boost::asio::placeholders::error)));
		}

		void ConnectionImpl::closeReserve(const size_t timeAfterMs)
		{
			_closeReserveTimer.expires_from_now(
				boost::posix_time::milliseconds(timeAfterMs));

			_closeReserveTimer.async_wait(_strand.wrap(
				boost::bind(&ConnectionImpl::onCloseReserveTime
					, this
					, boost::asio::placeholders::error)));
		}

		void ConnectionImpl::onExpireTime(const boost::system::error_code& ec)
		{
			if (!ec && _status == status::open)
			{
				if (_eventReceiver) {
					_eventReceiver->onExpiredSession(shared_from_this());
				}
			}
		}

		void ConnectionImpl::close()
		{
			boost::system::error_code ec;

			// socket close
			_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

			if (!ec)
			{
				set_status(Connection::status::close);

				// timer close.
				_keepTimer.cancel();
				_closeReserveTimer.cancel();

				_eventReceiver->onClose(shared_from_this());
			}
			else
			{
				LOGD << "status check : " << ec.message().data();
			}
		}

		void ConnectionImpl::onCloseReserveTime(const boost::system::error_code& ec)
		{
			if (!ec && _status == status::open)
			{
				close();
			}
		}

		unsigned int ConnectionImpl::incReadHandlerPendingCount()
		{
			/*return _readHandlerPending.fetch_add(1, std::memory_order_relaxed);*/
			return _readHandlerPending++;
		}

		unsigned int ConnectionImpl::decReadHandlerPendingCount()
		{
			/*return _readHandlerPending.fetch_sub(1, std::memory_order_relaxed);*/
			return _readHandlerPending--;
		}

		EventReceiver * ConnectionImpl::getReceiver()
		{
			return _eventReceiver;
		}

		MsgUserManip * ConnectionImpl::getMsgManip()
		{
			return _msgManipulator;
		}


	};//namespace Net
};//namespace MLN
