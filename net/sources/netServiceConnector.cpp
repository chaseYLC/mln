#include "stdafx.h"
#include "NetServiceConnector.h"

#include "ConnectionImpl.h"
#include "EventReceiver.h"


namespace MLN
{
	namespace Net
	{
		NetServiceConnector::NetServiceConnector(serviceInitParams &params)
			: NetServiceImpl(params)
		{
		}


		NetServiceConnector::~NetServiceConnector()
		{
		}

		void NetServiceConnector::acceptWait(const std::string& addr, const std::string& port
			, size_t workerThreadsCount
			, const size_t keepAliveMilliseconds)
		{
			assert(false && "connector can't accept");
		}

		void NetServiceConnector::connectWait(const std::string& addr, const std::string& port)
		{
			using TCP = boost::asio::ip::tcp;

			std::string ipAddr;
			if (true == addr.empty()){
				ipAddr = "127.0.0.1";
			}
			else{
				ipAddr = addr;
			}

			TCP::resolver resolver(_ios);
			TCP::resolver::query quary(ipAddr.c_str(), port);
			TCP::resolver::iterator endpoint_iter = resolver.resolve(quary);

			if (TCP::resolver::iterator() != endpoint_iter)
			{
				LOGD << "Trying " << endpoint_iter->endpoint();

				//// Set a deadline for the connect operation.
				//deadline_.expires_from_now(boost::posix_time::seconds(60));

				_conn = Connection::ptr(new ConnectionImpl(this, _ios, _packetHeaderManip), ConnectionImpl::destruct);

				// Start the asynchronous connect operation.
				/*_conn->socket().async_connect(endpoint_iter->endpoint()
					, boost::bind(&NetServiceConnector::connect_handler
					, this, _1, endpoint_iter));*/

				m_connecting = true;

				_conn->socket().async_connect(endpoint_iter->endpoint()
					, _strand.wrap(boost::bind(&NetServiceConnector::connect_handler, this, boost::asio::placeholders::error, endpoint_iter)));

				expireTimerReady();
			}
			else
			{
				assert(false && "There are no more endpoints to try. Shut down the client.");
			}
		}

		void NetServiceConnector::connect_handler(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator endpoint_iter)
		{
			m_connecting = false;

			// The async_connect() function automatically opens the socket at the start
			// of the asynchronous operation. If the socket is closed at this time then
			// the timeout handler must have run first.
			if (false == _conn->socket().is_open()){
				LOGD << "Connect timed out";

				_conn->socket().close();
				_receiver.onConnectFailed(_conn);
			}
			else if (ec)// Check if the connect operation failed
			{
				LOGE << "Connect error: " << ec.message();

				_conn->socket().close();

				_receiver.onConnectFailed(_conn);
				/*_updater.cancel();*/
			}
			else{
				LOGD << "Connected to " << endpoint_iter->endpoint();

				_conn->start_connect();

				_receiver.setSession(_conn);

				_receiver.onConnect(_conn);
			}
		}

		size_t NetServiceConnector::getKeepAliveTimeMs() const
		{
			return 0;
		}

		boost::weak_ptr<Net::Connection> NetServiceConnector::getSession() const
		{
			return _receiver.getSession();
		}

	};//namespace Net


};//namespace MLN