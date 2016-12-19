#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

namespace MLN
{
	namespace Base
	{
		class MessageBuffer;
	}

	namespace Net
	{
		class NetService;

		class Connection
		{
		public:
			using ptr = boost::shared_ptr<Connection>;

			enum class status : unsigned char {
				open = 0,
				closing = 1,
				close = 2,
			};

		public:
			virtual void set_status(const status s) = 0;
			virtual status get_status() const = 0;
			virtual boost::asio::ip::tcp::socket& socket() = 0;
			virtual boost::asio::strand& strand() = 0;
			virtual NetService* owner() = 0;

			virtual void start_accept() = 0;
			virtual void start_connect() = 0;
			virtual void send(MLN::Base::MessageBuffer& msg) = 0;
			virtual void sendRaw(void* sendBuffer, const size_t sendSize) = 0;
			virtual void sendPacket(void* sendBuffer, const size_t sendSize) = 0;

			virtual void setTag(void* tag) = 0;
			virtual void* getTag() const = 0;

			virtual size_t getIdentity() const = 0;

			virtual void closeReserve(const size_t timeAfterMs) = 0;
			virtual unsigned int incReadHandlerPendingCount() = 0;
			virtual unsigned int decReadHandlerPendingCount() = 0;

			virtual bool isSyncEncryptionKey(const uint8_t encType) const{ return true; };
			virtual void setSyncEncryptionKey(const uint8_t encType, const bool sync){};

		};
	};//namespace Net

};//namespace MLN
