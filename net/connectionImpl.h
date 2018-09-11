#ifndef _MLN_NET_CONNECTION_IMPL_H_
#define _MLN_NET_CONNECTION_IMPL_H_


#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>


#include "connection.h"
#include "memoryPool.h"
#include "messageBuffer.h"
#include "netService.h"
#include "messageProcedure.h"

namespace MLN
{
	namespace Net
	{
		class NetService;
		class EventReceiver;

		class ConnectionImpl
			: public Connection
			, public boost::enable_shared_from_this< ConnectionImpl >
			, public MemoryPool< ConnectionImpl >
		{
		public:
			ConnectionImpl(NetService* owner, boost::asio::io_service& ios
				, MessageProcedure *msgProc, EventReceiver *evntReceiver
				, const size_t keepAliveTimeMs
				, const size_t connectionID
				);

			void set_status(const status s) override;
			status get_status() const override;
			boost::asio::ip::tcp::socket& socket() override;
			boost::asio::strand& strand() override;

			void start_accept() override;
			void start_connect() override;
			void send(MessageBuffer& msg) override;
			void sendRaw(void* sendBuffer, const size_t sendSize) override;
			void sendPacket(void* sendBuffer, const size_t sendSize) override;

			void  setTag(void* tag) override;
			void* getTag() const override;
			size_t getIdentity() const  override;
			void setServiceID(const size_t id) override;
			size_t getServiceID() const override;
			size_t getConnectionID() const override;

			bool isSyncEncryptionKey(const uint8_t encType) const override;
			void setSyncEncryptionKey(const uint8_t encType, const bool sync) override;

			void closeReserve(const size_t timeAfterMs) override;
			unsigned int incReadHandlerPendingCount() override;
			unsigned int decReadHandlerPendingCount() override;

			EventReceiver * getReceiver() override;
			MsgUserManip * getMsgManip() override;

		private:
			void read_handler(const boost::system::error_code& ec, size_t bytes_transferred);
			void write_handler(const boost::system::error_code& ec, size_t bytes_transferred);

			void renewExpireTime();
			void onExpireTime(const boost::system::error_code& ec);
			void onCloseReserveTime(const boost::system::error_code& ec);
			void close();
			void closing(boost::asio::socket_base::shutdown_type what);

			status _status;
			boost::asio::io_service&		_ios;
			boost::asio::strand _strand;
			boost::asio::ip::tcp::socket _socket;
			boost::asio::deadline_timer _keepTimer;
			boost::asio::deadline_timer _closeReserveTimer;
			MessageBuffer::Ptr _msg;
			MessageBuffer::Ptr _msgRecv;
			void* _tag;
			size_t _identity;
			size_t _netServiceID;
			size_t _connectionID;

			bool _syncEncryptionKey[UINT8_MAX];


			size_t _keepAliveTime;

			unsigned int _readHandlerPending = 0;
			size_t _postRetryCount = 0;

			MessageProcedure * _msgProc = nullptr;
			EventReceiver * _eventReceiver = nullptr;
			MsgUserManip *_msgManipulator = nullptr;
		};
	};//namespace Net
};//namespace MLN


#endif