#pragma once

#include "netServiceImpl.h"

namespace mln
{
	class NetServiceConnector
		: public NetServiceImpl
	{
		friend class ConnectionImpl;
		friend class NetService;

	public:
		NetServiceConnector(ServiceParams& params, ConnectorUserParams& userParams);
		~NetServiceConnector();

	private:
		bool connect_handler(const boost::system::error_code& ec
			, boost::asio::ip::tcp::resolver::iterator endpoint_iter, Connection::sptr conn, const uint16_t remainCnt);

		bool connect_handler_sync(boost::asio::ip::tcp::resolver::iterator endpoint_iter, Connection::sptr conn);

		bool connectImplement(const std::string& addr, const std::string& port, const uint16_t sessionCnt, const size_t connectionID = 0);
		Connection::sptr connectImplement_sync(const std::string& addr, const std::string& port, const size_t connectionID = 0);
		bool setEndPoint(const std::string& addr, const std::string& port);
		Connection::sptr createConnectSession(const size_t connectionID);

	public:
		void setTargetServer(const std::string& addr, const std::string& port);
		void connectWait(const uint16_t sessionCnt = 1, size_t workerThreadsCount = 1, const size_t connectionID = 0);
		Connection::sptr connect();
	private:
		std::string m_targetAddr;
		std::string m_targetPort;

		// for 
		boost::asio::ip::tcp::resolver* m_resolver = nullptr;
		boost::asio::ip::tcp::resolver::query* m_quary = nullptr;
		boost::asio::ip::tcp::resolver::iterator* m_endpoint_iter = nullptr;
	};
};//namespace mln