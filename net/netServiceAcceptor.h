#pragma once

#include "netServiceImpl.h"

namespace mln
{
	class NetServiceAcceptor
		: public NetServiceImpl
	{
		friend class ConnectionImpl;
		friend class NetService;

	public:
		NetServiceAcceptor(ServiceParams& params);
		~NetServiceAcceptor();

	private:

		virtual void acceptWait(const std::string& addr, const std::string& port
			, size_t workerThreadsCount);

		void accept_handler(const boost::system::error_code& ec, Connection::sptr conn);

	private:
		/*Connection::sptr _conn;*/
		boost::asio::ip::tcp::acceptor	_acceptorSocket;
	};
};//namespace mln