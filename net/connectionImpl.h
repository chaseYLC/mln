#pragma once

#include "connection.h"
#include "memoryPool.h"
#include "circularStream.h"

namespace mln
{
	class EventReceiver;
	class MessageProcedure;
	class NetService;

	class ConnectionImpl
		: public Connection
		, public std::enable_shared_from_this< ConnectionImpl >
		, public MemoryPool< ConnectionImpl >
	{
	public:
		static std::shared_ptr< ConnectionImpl > create(NetService* owner
			, boost::asio::io_context& ios
			, MessageProcedure* msgProc
			, EventReceiver* evntReceiver
			, const size_t keepAliveTimeMs
			, const size_t connectionID
		);

		static Connection::sptr createDummyTestConnection(boost::asio::io_context& ios);
		static const int RECV_BUFFER_SIZE = 8192;

	private:
		ConnectionImpl(NetService* owner, boost::asio::io_context& ios
			, MessageProcedure* msgProc, EventReceiver* evntReceiver
			, const size_t keepAliveTimeMs
			, const size_t connectionID
		);

		ConnectionImpl(boost::asio::io_context& ios);

	public:
		void set_status(const status s) override;
		status get_status() const override;
		boost::asio::ip::tcp::socket& socket() override;
		boost::asio::io_context::strand& strand() override;

		void start_accept() override;
		void start_connect() override;
		void send(CircularStream::Ptr msg) override;
		void sendRaw(void* sendBuffer, const size_t sendSize) override;
		void sendPacket(void* sendBuffer, const size_t sendSize, const bool writeHeader) override;

		void  setTag(void* tag) override;
		void* getTag() const override;
		virtual void setUser(std::shared_ptr<UserBasis> user) override;
		virtual std::shared_ptr<UserBasis> getUser() const override;
		size_t getIdentity() const  override;
		void setServiceID(const size_t id) override;
		size_t getServiceID() const override;
		size_t getConnectionID() const override;

		bool isSyncEncryptionKey(const uint8_t encType) const override;
		void setSyncEncryptionKey(const uint8_t encType, const bool sync) override;

		void closeReserve(const size_t timeAfterMs) override;
		void closeReserveWithPost(const size_t timeAfterMs, std::function<void(void)> post) override;
		unsigned int incReadHandlerPendingCount() override;
		unsigned int decReadHandlerPendingCount() override;

		EventReceiver* getReceiver() override;
		MsgUserManip* getMsgManip() override;

	private:
		void read_handler(const boost::system::error_code& ec, size_t bytes_transferred);
		void write_handler(const boost::system::error_code& ec, size_t bytes_transferred, CircularStream::Ptr sendBuffer);

		void renewExpireTime();
		void onExpireTime(const boost::system::error_code& ec);
		void onCloseReserveTime(const boost::system::error_code& ec, std::function<void(void)> post);
		void close();
		void closing(boost::asio::socket_base::shutdown_type what);

		status _status;
		boost::asio::io_context& _ios;
		boost::asio::io_context::strand _strand;
		boost::asio::ip::tcp::socket _socket;
		boost::asio::deadline_timer _keepTimer;
		boost::asio::deadline_timer _closeReserveTimer;
		CircularStream::Ptr _msg;
		char _recvBuffer[RECV_BUFFER_SIZE];
		std::shared_ptr<UserBasis> _spUser;

		void* _tag = nullptr;
		size_t _identity;
		size_t _netServiceID;
		size_t _connectionID;

		bool _syncEncryptionKey[UINT8_MAX];


		size_t _keepAliveTime;

		unsigned int _readHandlerPending = 0;
		size_t _postRetryCount = 0;

		MessageProcedure* _msgProc = nullptr;
		EventReceiver* _eventReceiver = nullptr;
		MsgUserManip* _msgManipulator = nullptr;
	};
};//namespace mln
