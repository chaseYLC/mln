#include "stdafx.h"
#include "netServiceConnector.h"

#include "connectionImpl.h"
#include "eventReceiver.h"
#include "logManager.h"


namespace mln
{
	NetServiceConnector::NetServiceConnector(ServiceParams& params, ConnectorUserParams& userParams)
		: NetServiceImpl(
			params.ios
			, params.serviceUpdateTimeMs
			, params.keepAliveTimeMs
			, &params.receiver
			, params.manip
			, params.msgParser)
	{
		setTargetServer(userParams.addr, userParams.port);
	}

	NetServiceConnector::~NetServiceConnector()
	{
		delete(m_quary);
		delete(m_endpoint_iter);
		delete(m_resolver);
	}

	void NetServiceConnector::setTargetServer(const std::string& addr, const std::string& port)
	{
		m_targetAddr = addr;
		m_targetPort = port;
	}

	Connection::sptr NetServiceConnector::connect()
	{
		return connectImplement_sync(m_targetAddr, m_targetPort);
	}

	void NetServiceConnector::connectWait(const uint16_t sessionCnt /*= 1*/, size_t workerThreadsCount /*= 1*/, const size_t connectionID /*= 0*/)
	{
		if (0 < workerThreadsCount) {
			NetService::addWorkerThreads(workerThreadsCount, _ios);
		}

		connectImplement(m_targetAddr, m_targetPort, sessionCnt, connectionID);
	}

	bool NetServiceConnector::connectImplement(const std::string& addr, const std::string& port, const uint16_t sessionCnt, const size_t connectionID /*= 0*/)
	{
		assert(0 < sessionCnt);

		if (false == setEndPoint(addr, port)) {
			return false;
		}

		try {

			auto conn = createConnectSession(connectionID);

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			conn->socket().async_connect((*m_endpoint_iter)->endpoint()
				, boost::asio::bind_executor(_strand, boost::bind(
					&NetServiceConnector::connect_handler
					, this
					, boost::asio::placeholders::error
					, (*m_endpoint_iter), conn, sessionCnt - 1)));
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			expireTimerReady();
		}//try
		catch (std::exception& e)
		{
			LOGE("Connection failed. Exception:{}", e.what());
			return false;
		}

		return true;
	}

	Connection::sptr NetServiceConnector::connectImplement_sync(const std::string& addr, const std::string& port, const size_t connectionID /*= 0*/)
	{
		if (false == setEndPoint(addr, port)) {
			return Connection::sptr();
		}

		try {
			auto conn = createConnectSession(connectionID);

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			conn->socket().connect((*m_endpoint_iter)->endpoint());
			if (false == connect_handler_sync((*m_endpoint_iter), conn)) {
				LOGE("failed makeSession. connectionID : {}", connectionID);
				return nullptr;
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			expireTimerReady();

			return conn;
		}
		catch (std::exception& e)
		{
			LOGE("Connection failed. Exception: {}", e.what());
			return nullptr;
		}
	}

	bool NetServiceConnector::setEndPoint(const std::string& addr, const std::string& port)
	{
		using TCP = boost::asio::ip::tcp;

		std::string ipAddr;
		if (true == addr.empty()) {
			ipAddr = "127.0.0.1";
		}
		else {
			ipAddr = addr;
		}

		Connection::sptr conn;

		try {
			if (nullptr == m_resolver) {
				m_resolver = new TCP::resolver(_ios);
				m_quary = new TCP::resolver::query(ipAddr.c_str(), port);
				m_endpoint_iter = new TCP::resolver::iterator;
				*m_endpoint_iter = (*m_resolver).resolve(*m_quary);
			}

			if (*m_endpoint_iter == TCP::resolver::iterator()) {
				assert(false && "There are no more endpoints to try. Shut down the client.");
				return false;
			}
		}
		catch (std::exception& e)
		{
			LOGE("setEndPoint failed. Exception: {}", e.what());
			return false;
		}

		auto endPoint = (*m_endpoint_iter)->endpoint();
		LOGD("Trying {}/{}", endPoint.address().to_string(), endPoint.port());
		return true;
	}

	Connection::sptr NetServiceConnector::createConnectSession(const size_t connectionID)
	{
		using TCP = boost::asio::ip::tcp;

		auto conn = ConnectionImpl::create(this, _ios, _msgProc, &_eventReceiver, _keepAliveTimeMs, connectionID);
		conn->setServiceID(getIndex());

		conn->socket().open((*m_endpoint_iter)->endpoint().protocol());

		conn->socket().set_option(boost::asio::socket_base::linger(true, 0));
		conn->socket().set_option(TCP::no_delay(true));
		conn->socket().set_option(TCP::acceptor::reuse_address(true));

		return conn;
	}

	bool NetServiceConnector::connect_handler(const boost::system::error_code& ec
		, boost::asio::ip::tcp::resolver::iterator endpoint_iter
		, Connection::sptr conn
		, const uint16_t remainCnt)
	{
		// The async_connect() function automatically opens the socket at the start
		// of the asynchronous operation. If the socket is closed at this time then
		// the timeout handler must have run first.
		if (false == conn->socket().is_open()) {
			LOGD("Connect timed out");
			conn->socket().close();
			_eventReceiver.onConnectFailed(conn);
			return false;
		}

		if (ec)// Check if the connect operation failed
		{
			LOGE("Connect error: {}", ec.message());
			conn->socket().close();
			_eventReceiver.onConnectFailed(conn);
			return false;
		}

		auto endPoint = (*m_endpoint_iter)->endpoint();
		LOGD("Connected to {}/{}", endPoint.address().to_string(), endPoint.port());

		conn->start_connect();
		_eventReceiver.onConnect(conn);
		/*conn->strand().post(boost::bind(_eventReceiver->onConnect
		, conn));*/


		if (0 < remainCnt) {
			using TCP = boost::asio::ip::tcp;

			try {
				conn = createConnectSession(0);
				conn->socket().async_connect(endpoint_iter->endpoint()
					, boost::asio::bind_executor(_strand
						, boost::bind(
							&NetServiceConnector::connect_handler
							, this
							, boost::asio::placeholders::error
							, endpoint_iter, conn, remainCnt - 1)));
			}//try
			catch (std::exception& e)
			{
				LOGE("Connection failed. Exception: {}", e.what());
				return false;
			}
		}

		return true;
	}

	bool NetServiceConnector::connect_handler_sync(
		boost::asio::ip::tcp::resolver::iterator endpoint_iter
		, Connection::sptr conn)
	{
		auto endPoint = (endpoint_iter)->endpoint();
		LOGD("Connected to {}/{}", endPoint.address().to_string(), endPoint.port());

		conn->start_connect();

		_eventReceiver.onConnect(conn);
		/*conn->strand().post(boost::bind(_eventReceiver->onConnect
		, conn));*/

		return true;
	}

};//namespace mln