#include "stdafx.h"
#include "connectionImpl.h"

#include <atomic>
#include <boost/bind.hpp>
#include <boost/asio/write.hpp>
#include "circularStream.h"
#include "messageProcedure.h"
#include "netServiceAcceptor.h"
#include "logManager.h"

#include "user/userBasis.h"

namespace mln
{
	std::atomic< size_t > s_identitySeed = { 1 };

	Connection::~Connection()
	{
	}

	// for test
	Connection::sptr ConnectionImpl::createDummyTestConnection(boost::asio::io_context& ios)
	{
		return Connection::sptr(new ConnectionImpl(ios), ConnectionImpl::destruct);
	}

	ConnectionImpl::ConnectionImpl(boost::asio::io_context& ios)
		: _ios(ios)
		, _socket(ios)
		, _strand(ios)
		, _keepTimer(ios)
		, _closeReserveTimer(ios)
	{
	}

	ConnectionImpl::ConnectionImpl(NetService* owner, boost::asio::io_context& ios
		, MessageProcedure* msgProc, EventReceiver* evntReceiver
		, const size_t keepAliveTimeMs
		, const size_t connectionID
	)
		: _ios(ios)
		, _socket(ios)
		, _strand(ios)
		, _status(status::close)
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

	std::shared_ptr< ConnectionImpl > ConnectionImpl::create(NetService* owner
		, boost::asio::io_context& ios
		, MessageProcedure* msgProc
		, EventReceiver* evntReceiver
		, const size_t keepAliveTimeMs
		, const size_t connectionID
	)
	{
		return std::shared_ptr< ConnectionImpl >(new ConnectionImpl(owner
			, ios
			, msgProc
			, evntReceiver
			, keepAliveTimeMs
			, connectionID
		), ConnectionImpl::destruct);
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

	//boost::asio::strand& ConnectionImpl::strand()
	boost::asio::io_context::strand& ConnectionImpl::strand()
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

	void ConnectionImpl::setUser(std::shared_ptr<UserBasis> user)
	{
		_spUser = user;
	}

	std::shared_ptr<UserBasis> ConnectionImpl::getUser() const
	{
		return _spUser;
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
		_msg = CircularStream::Ptr(new CircularStream(_msgManipulator), CircularStream::destruct);

		_socket.set_option(boost::asio::ip::tcp::no_delay(true));
		_socket.set_option(boost::asio::socket_base::linger(true, 0));

		_socket.async_read_some(
			boost::asio::buffer(_recvBuffer, sizeof(_recvBuffer))
			, boost::asio::bind_executor(_strand
				, boost::bind(&ConnectionImpl::read_handler
					, shared_from_this()
					, boost::asio::placeholders::error
					, boost::asio::placeholders::bytes_transferred)));

		_status = status::open;

		renewExpireTime();
	}

	void ConnectionImpl::start_connect()
	{
		_msg = CircularStream::Ptr(new CircularStream(_msgManipulator), CircularStream::destruct);

		_socket.set_option(boost::asio::ip::tcp::no_delay(true));
		_socket.set_option(boost::asio::socket_base::linger(true, 0));

		_socket.async_read_some(
			boost::asio::buffer(_recvBuffer, sizeof(_recvBuffer))
			, boost::asio::bind_executor(_strand
				, boost::bind(&ConnectionImpl::read_handler
					, shared_from_this()
					, boost::asio::placeholders::error
					, boost::asio::placeholders::bytes_transferred)));

		_status = status::open;

		renewExpireTime();
	}

	void ConnectionImpl::send(CircularStream::Ptr msg)
	{
		msg->facilitate();

		boost::asio::async_write(
			_socket
			, boost::asio::buffer(msg->data(), msg->size())
			, boost::asio::bind_executor(_strand
				, boost::bind(&ConnectionImpl::write_handler
					, shared_from_this()
					, boost::asio::placeholders::error
					, boost::asio::placeholders::bytes_transferred
					, msg
				))
		);
	}

	void ConnectionImpl::sendRaw(void* sendBuffer, const size_t sendSize)
	{
		auto msg = CircularStream::Ptr(new CircularStream(), CircularStream::destruct);

		boost::asio::async_write(
			_socket
			, boost::asio::buffer(sendBuffer, sendSize)
			, boost::asio::bind_executor(_strand
				, boost::bind(&ConnectionImpl::write_handler
					, shared_from_this()
					, boost::asio::placeholders::error
					, boost::asio::placeholders::bytes_transferred
					, msg
				))
		);
	}

	void ConnectionImpl::sendPacket(void* sendBuffer, const size_t sendSize, const bool writeHeader)
	{
		CircularStream::Ptr msg = CircularStream::Ptr(new CircularStream(_msgManipulator, writeHeader), CircularStream::destruct);

		msg->write((char*)sendBuffer, sendSize);
		this->send(msg);
	}

	void ConnectionImpl::read_handler(const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (!ec)
		{
			fmt::print("recv data. size:{}\n", bytes_transferred);

			try {
				_msg->write(_recvBuffer, bytes_transferred);
			}
			catch (std::exception& e) {
				LOGW("network data was received but could not be processed and the queue was full.\
Increase the size of the CircularStream's size or reduce the number of requests");
				closeReserve(0);
				return;
			}
			
			incReadHandlerPendingCount();

			boost::asio::post(boost::asio::bind_executor(_strand
				, boost::bind(&MessageProcedure::dispatch
					, _msgProc
					, shared_from_this()
					, _msg)));

			_socket.async_read_some(
				boost::asio::buffer(_recvBuffer, sizeof(_recvBuffer))
				, boost::asio::bind_executor(_strand
					, boost::bind(&ConnectionImpl::read_handler
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

					boost::asio::post(boost::asio::bind_executor(_strand
						, boost::bind(&ConnectionImpl::read_handler
							, shared_from_this()
							, ec
							, bytes_transferred)));

					return;
				}

				LOGW("increase postRetryCount Limit. msg value : {}, msg string:{}, , pending:{}"
					, std::to_string(ec.value()), ec.message(), std::to_string(_readHandlerPending));
			}

			closing(boost::asio::ip::tcp::socket::shutdown_receive);
		}
	}


	void ConnectionImpl::closing(boost::asio::socket_base::shutdown_type what)
	{
		boost::system::error_code ec;
		_socket.shutdown(what, ec);

		if (ec) {
			LOGW("error in closing(). msg:{}", ec.message());
		}

		if (_status == status::open){
			_status = status::closing;

			_keepTimer.cancel(ec);
			_closeReserveTimer.cancel(ec);

			boost::asio::post(
				boost::asio::bind_executor(_strand
					, boost::bind(&ConnectionImpl::close
						, shared_from_this())));
		}
	}


	void ConnectionImpl::write_handler(const boost::system::error_code& ec, size_t bytes_transferred, CircularStream::Ptr sendBuffer)
	{
		if (!ec) {
			fmt::print("write data. size:{}\n", bytes_transferred);
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

		_keepTimer.async_wait(
			boost::asio::bind_executor(_strand
				, boost::bind(&ConnectionImpl::onExpireTime
					, this
					, boost::asio::placeholders::error)));
	}

	void ConnectionImpl::closeReserve(const size_t timeAfterMs)
	{
		_closeReserveTimer.expires_from_now(
			boost::posix_time::milliseconds(timeAfterMs));

		_closeReserveTimer.async_wait(boost::asio::bind_executor(_strand
			, boost::bind(&ConnectionImpl::onCloseReserveTime
				, this
				, boost::asio::placeholders::error, nullptr)));
	}

	void ConnectionImpl::closeReserveWithPost(const size_t timeAfterMs, std::function<void(void)> post)
	{
		_closeReserveTimer.expires_from_now(
			boost::posix_time::milliseconds(timeAfterMs));

		_closeReserveTimer.async_wait(boost::asio::bind_executor(_strand
			, boost::bind(&ConnectionImpl::onCloseReserveTime
				, this
				, boost::asio::placeholders::error, post)));
	}

	void ConnectionImpl::onExpireTime(const boost::system::error_code& ec)
	{
		if (ec && ec != boost::asio::error::operation_aborted) {
			LOGW("error in onExpireTime(). msg:{}", ec.message());
		}
		if (!ec && _status == status::open){
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

		if (ec) {
			LOGD("error in close(). status check : {}", ec.message());
		}

		if (status::close != get_status() ){
			set_status(Connection::status::close);

			// timer close.
			_keepTimer.cancel(ec);
			_closeReserveTimer.cancel(ec);

			_eventReceiver->onClose(shared_from_this());
		}
	}

	void ConnectionImpl::onCloseReserveTime(const boost::system::error_code& ec, std::function<void(void)> post)
	{
		if (ec) {
			LOGD("error in onCloseReserveTime(). status check : {}", ec.message());
		}
		if (_status == status::open){
			close();

			if (post) {
				post();
			}
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

	EventReceiver* ConnectionImpl::getReceiver()
	{
		return _eventReceiver;
	}

	MsgUserManip* ConnectionImpl::getMsgManip()
	{
		return _msgManipulator;
	}


};//namespace mln
