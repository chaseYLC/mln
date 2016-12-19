#include "stdafx.h"
#include "ConnectionImpl.h"

#include <atomic>
#include <Base/Base/include/messageBuffer.h>
#include <boost/bind.hpp>
#include <boost/asio/write.hpp>
#include "netServiceAcceptor.h"

namespace MLN
{
	namespace Net
	{
		std::atomic< size_t > s_identitySeed = { 0 };

		ConnectionImpl::ConnectionImpl(NetService* owner, boost::asio::io_service& ios
			, MLN::Base::customHeaderManipulator *manip)
			: _owner(owner)
			, _ios(ios)
			, _socket(ios)
			, _strand(ios)
			, _status(status::close)
			, _tag(nullptr)
			, _msgManipulator(manip)
			, _keepTimer(ios)
			, _closeReserveTimer(ios)
			, _postRetryCount(0)
		{
			memset(_syncEncryptionKey, 0, sizeof(_syncEncryptionKey));

			_identity = s_identitySeed.fetch_add(1, std::memory_order_relaxed);

			_keepAliveTime = _owner->getKeepAliveTimeMs();

			renewExpireTime();
		}

		ConnectionImpl::~ConnectionImpl()
		{
			_owner = nullptr;
			_tag = nullptr;
			_status = status::close;
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

		NetService* ConnectionImpl::owner()
		{
			return _owner;
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
			_msg = MLN::Base::MessageBuffer::ptr(new MLN::Base::MessageBuffer(_msgManipulator), MLN::Base::MessageBuffer::destruct);

			_socket.set_option(boost::asio::ip::tcp::no_delay(true));

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
			_msg = MLN::Base::MessageBuffer::ptr(new MLN::Base::MessageBuffer(_msgManipulator), MLN::Base::MessageBuffer::destruct);

			_socket.set_option(boost::asio::ip::tcp::no_delay(true));

			_socket.async_read_some(
				boost::asio::buffer(_msg->enableBuffer(), _msg->remainWriteSize())
				, _strand.wrap(boost::bind(&ConnectionImpl::read_handler
				, shared_from_this()
				, boost::asio::placeholders::error
				, boost::asio::placeholders::bytes_transferred)));

			_status = status::open;

			renewExpireTime();
		}

		void ConnectionImpl::send(MLN::Base::MessageBuffer& msg)
		{
			msg.facilitate();

			/*_socket.async_write_some(boost::asio::buffer(msg.data(), msg.size())
				, _strand.wrap(boost::bind(&ConnectionImpl::write_handler
				, shared_from_this()
				, boost::asio::placeholders::error
				, boost::asio::placeholders::bytes_transferred)));*/

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

			/*_socket.async_write_some(boost::asio::buffer(sendBuffer, sendSize)
				, _strand.wrap(boost::bind(&ConnectionImpl::write_handler
				, shared_from_this()
				, boost::asio::placeholders::error
				, boost::asio::placeholders::bytes_transferred)));*/
		}

		void ConnectionImpl::sendPacket(void* sendBuffer, const size_t sendSize)
		{
			MLN::Base::MessageBuffer msg(_msgManipulator, 0);
			msg.write((char*)sendBuffer, sendSize);
			this->send(msg);
		}


		void ConnectionImpl::dispatchReadMsg()
		{
			owner()->dispatch(shared_from_this(), _msgRecv);
			_msgRecv.reset();
		}

		void ConnectionImpl::read_handler(const boost::system::error_code& ec, size_t bytes_transferred)
		{
			if (!ec)
			{
				_msg->write(bytes_transferred);

				MLN::Base::MessageBuffer::ptr msgRecv = _msg;
				_msg = MLN::Base::MessageBuffer::ptr(new MLN::Base::MessageBuffer(_msgManipulator), MLN::Base::MessageBuffer::destruct);

				/*((NetServiceAcceptor*)_owner)->msgEnque(shared_from_this(), msgRecv);*/
				incReadHandlerPendingCount();
				strand().post(std::bind(
					&NetServiceImpl::dispatch
					, _owner
					, shared_from_this()
					, msgRecv));


				_socket.async_read_some(
					boost::asio::buffer(_msg->enableBuffer(), _msg->remainWriteSize())
					, _strand.wrap(boost::bind(&ConnectionImpl::read_handler
					, shared_from_this()
					, boost::asio::placeholders::error
					, boost::asio::placeholders::bytes_transferred)));

				renewExpireTime();


				//_msg->write(bytes_transferred);
				//_msgRecv.reset();
				//_msgRecv = _msg;

				//_msg = message::ptr(new message(_msgManipulator), message::destruct);

				//incReadHandlerPendingCount();
				//strand().post(std::bind(&ConnectionImpl::dispatchReadMsg, this));

				//// ready ready continue.
				//_socket.async_read_some(
				//	boost::asio::buffer(_msg->enableBuffer(), _msg->remainWriteSize())
				//	, _strand.wrap(boost::bind(&ConnectionImpl::read_handler
				//	, shared_from_this()
				//	, boost::asio::placeholders::error
				//	, boost::asio::placeholders::bytes_transferred)));

				//renewExpireTime();
			}
			else
			{
				const unsigned int readHandlerPendingCount = _readHandlerPending.load();

				if (0 < readHandlerPendingCount)
				{
					/*LOGD << "occured upset of packet-seq. post this into queue.";*/

					if (5 > _postRetryCount++){
						_ios.post(_strand.wrap(boost::bind(
							&ConnectionImpl::read_handler
							, shared_from_this()
							, ec
							, bytes_transferred)));

						return;
					}

					LOGE << "increase postRetryCount Limit.";
				}

				boost::system::error_code ec_;
				_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec_);
				if (!ec_ && _status == status::open)
				{
					_status = status::closing;
					_owner->close(shared_from_this());

					_keepTimer.cancel();
					_closeReserveTimer.cancel();
				}
			}
		}

		void ConnectionImpl::write_handler(const boost::system::error_code& ec, size_t bytes_transferred)
		{
			if (ec)
			{
				boost::system::error_code ec_;
				_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec_);
				if (!ec_ && _status == status::open)
				{
					_status = status::closing;
					_owner->close(shared_from_this());
				}

				return;
			}

			renewExpireTime();
		}


		void ConnectionImpl::renewExpireTime()
		{
			if (0 == _keepAliveTime){
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
#ifdef AUTO_SESSION_CLOSE_ON_EXPIRE_TIME_IN_MLNNET
			if (_status == status::open)
			{
				_status = status::closing;
				_owner->close(shared_from_this());
			}
#else
			if (!ec && _status == status::open)
			{
				_owner->getReceiver().onExpiredSession(shared_from_this());
			}
#endif

		}

		void ConnectionImpl::onCloseReserveTime(const boost::system::error_code& ec)
		{
			if (!ec && _status == status::open)
			{
				_status = status::closing;
				_owner->close(shared_from_this());
			}
		}

		unsigned int ConnectionImpl::incReadHandlerPendingCount()
		{
			return _readHandlerPending.fetch_add(1, std::memory_order_relaxed);
		}

		unsigned int ConnectionImpl::decReadHandlerPendingCount()
		{
			return _readHandlerPending.fetch_sub(1, std::memory_order_relaxed);
		}


	};//namespace Net
};//namespace MLN
