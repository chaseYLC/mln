#pragma once

#include "NetServiceImpl.h"

namespace MLN{
	namespace Base{
		class RunningAlone;
	};
}

namespace MLN
{
	namespace Net
	{
		class NetServiceAcceptor
			: public NetServiceImpl
		{
			friend class ConnectionImpl;
			friend class NetService;

		public:
			NetServiceAcceptor(serviceInitParams &params);
			~NetServiceAcceptor();

			size_t getKeepAliveTimeMs() const override;

		private:

			void acceptWait(const std::string& addr, const std::string& port
				, size_t workerThreadsCount
				, const size_t keepAliveMilliseconds
				) override;

			void connectWait(const std::string& addr, const std::string& port) override;

			void accept_handler(const boost::system::error_code& ec);

			MLN::Base::RunningAlone * getRunningAloneCheckObject();

		private:
			boost::asio::ip::tcp::acceptor	_acceptor;
			size_t _keepAliveTimeMs;
			MLN::Base::RunningAlone *_runningAloneObj = nullptr;
		};
	};//namespace Net
};//namespace MLN