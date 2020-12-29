#pragma once

#include <functional>
#include <memory>
#include <boost/asio.hpp>
#include "messageBuffer.h"

namespace mln
{
	typedef int status_code;
	class status_codes
	{
	public:
#define _XRNET_SOCKET
#define DAT(a,b,c) const static status_code a=b;
#include "../mln_constants.dat"
#undef _XRNET_SOCKET
#undef DAT
	};

	class NetService;
	class EventReceiver;
	class UserBasis;

	class Connection
	{
	public:
		using sptr = std::shared_ptr<Connection>;
		using wptr = std::weak_ptr<Connection>;

		enum class status : unsigned char {
			open = 0,
			closing = 1,
			close = 2,
		};

	public:
		virtual ~Connection();

		virtual void set_status(const status s) = 0;
		virtual status get_status() const = 0;
		virtual boost::asio::ip::tcp::socket& socket() = 0;
		//virtual boost::asio::strand& strand() = 0;
		virtual boost::asio::io_context::strand& strand() = 0;

		virtual void start_accept() = 0;
		virtual void start_connect() = 0;
		virtual void send(MessageBuffer::Ptr msg) = 0;
		virtual void sendRaw(void* sendBuffer, const size_t sendSize) = 0;
		virtual void sendPacket(void* sendBuffer, const size_t sendSize, const bool writeHeader) = 0;

		virtual void setTag(void* tag) = 0;
		virtual void* getTag() const = 0;
		virtual void setUser(std::shared_ptr<UserBasis> user) = 0;
		virtual std::shared_ptr<UserBasis> getUser() const = 0;
		virtual void setServiceID(const size_t id) = 0;
		virtual size_t getServiceID() const = 0;
		virtual size_t getConnectionID() const = 0;

		virtual size_t getIdentity() const = 0;
		virtual bool isSyncEncryptionKey(const uint8_t encType) const { return true; };
		virtual void setSyncEncryptionKey(const uint8_t encType, const bool sync) {};

		virtual void closeReserve(const size_t timeAfterMs) = 0;
		virtual void closeReserveWithPost(const size_t timeAfterMs, std::function<void(void)> post) = 0;
		virtual unsigned int incReadHandlerPendingCount() = 0;
		virtual unsigned int decReadHandlerPendingCount() = 0;
		virtual EventReceiver* getReceiver() = 0;
		virtual MsgUserManip* getMsgManip() = 0;
	};

}//namespace mln
