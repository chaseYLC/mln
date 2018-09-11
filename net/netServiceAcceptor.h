#pragma once

#include "NetServiceImpl.h"

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
			NetServiceAcceptor(ServiceParams &params);
			~NetServiceAcceptor();

		private:

			virtual void acceptWait(const std::string& addr, const std::string& port
				, size_t workerThreadsCount);

			void accept_handler(const boost::system::error_code& ec, Connection::ptr conn);

		private:
			/*Connection::ptr _conn;*/
			boost::asio::ip::tcp::acceptor	_acceptorSocket;
		};
	};//namespace Net
};//namespace MLN