#pragma once

#include "NetServiceImpl.h"

namespace MLN
{
	namespace Net
	{
		class NetServiceConnector
			: public NetServiceImpl
		{
			friend class ConnectionImpl;
			friend class NetService;

		public:
			NetServiceConnector(serviceInitParams &params);
			~NetServiceConnector();

			size_t getKeepAliveTimeMs() const override;

		private:


			void acceptWait(const std::string& addr, const std::string& port
				, size_t workerThreadsCount
				, const size_t keepAliveMilliseconds) override;

			void connect_handler(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator endpoint_iter);

		public:
			void connectWait(const std::string& addr, const std::string& port) override;
			bool isConnecting()const{ return m_connecting; }
			boost::weak_ptr<Net::Connection> getSession() const;

		private:
			bool m_connecting = false;
		};
	};//namespace Net

};//namespace MLN