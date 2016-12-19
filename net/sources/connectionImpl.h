#pragma once

#include <atomic>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <Base/Base/include/memoryPool.h>
#include <Base/Base/include/messageBuffer.h>

#include "connection.h"
#include "NetService.h"
#include "messageProcedure.h"

namespace MLN
{
	namespace Net
	{
		class ConnectionImpl
			: public Connection
			, public boost::enable_shared_from_this< ConnectionImpl >
			, public MLN::Base::MemoryPool< ConnectionImpl >
		{
		public:
			ConnectionImpl(NetService* owner, boost::asio::io_service& ios
				, MLN::Base::customHeaderManipulator *manip);
			virtual ~ConnectionImpl();

			void set_status(const status s) override;
			status get_status() const override;
			boost::asio::ip::tcp::socket& socket() override;
			boost::asio::strand& strand() override;
			NetService* owner() override;

			void start_accept() override;
			void start_connect() override;
			void send(MLN::Base::MessageBuffer& msg) override;
			void sendRaw(void* sendBuffer, const size_t sendSize) override;
			void sendPacket(void* sendBuffer, const size_t sendSize) override;

			void  setTag(void* tag) override;
			void* getTag() const override;
			size_t getIdentity() const  override;

			bool isSyncEncryptionKey(const uint8_t encType) const override;
			void setSyncEncryptionKey(const uint8_t encType, const bool sync) override;

			void closeReserve(const size_t timeAfterMs) override;
			unsigned int incReadHandlerPendingCount() override;
			unsigned int decReadHandlerPendingCount() override;

		private:
			void read_handler(const boost::system::error_code& ec, size_t bytes_transferred);
			void write_handler(const boost::system::error_code& ec, size_t bytes_transferred);

			void renewExpireTime();
			void onExpireTime(const boost::system::error_code& ec);
			void onCloseReserveTime(const boost::system::error_code& ec);

			void dispatchReadMsg();

			status _status;
			boost::asio::io_service&		_ios;
			boost::asio::strand _strand;
			boost::asio::ip::tcp::socket _socket;
			NetService* _owner;
			boost::asio::deadline_timer _keepTimer;
			boost::asio::deadline_timer _closeReserveTimer;
			MLN::Base::MessageBuffer::ptr _msg;
			MLN::Base::MessageBuffer::ptr _msgRecv;
			void* _tag;
			size_t _identity;

			bool _syncEncryptionKey[UINT8_MAX];

			MLN::Base::customHeaderManipulator *_msgManipulator;
			size_t _keepAliveTime;

			std::atomic<unsigned int> _readHandlerPending = { 0 };
			size_t _postRetryCount = 0;
		};
	};//namespace Net
};//namespace MLN
