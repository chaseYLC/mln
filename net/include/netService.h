#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "container.h"
#include "eventReceiver.h"
#include "MessageProcedure.h"

namespace MLN
{
	namespace Net
	{
		class MessageBuffer;
		class Connection;
		class NetServiceAcceptor;
		class NetServiceConnector;

		class NetService :
			public Container,
			public MessageProcedure
		{
		public:
			using AcceptorPtrType = std::shared_ptr<NetServiceAcceptor>;
			using ConnectorPtrType = std::shared_ptr<NetServiceConnector>;
			using AcceptorContainerType = std::vector< AcceptorPtrType >;
			using ConnectorContainerType = std::vector< ConnectorPtrType >;
			using WeakConnectorType = std::weak_ptr<Net::NetServiceConnector>;
			using WeakAcceptorType = std::weak_ptr<Net::NetServiceAcceptor>;

		public:
			struct serviceInitParams{
				boost::asio::io_service& ios;
				EventReceiver& receiver;
				MessageProcedure::customMessageParser msgParser;
				MLN::Base::customHeaderManipulator* manip;
				size_t serviceUpdateTime;

				serviceInitParams(boost::asio::io_service& p_ios
					, EventReceiver& p_receiver
					, MessageProcedure::customMessageParser p_msgParser
					, MLN::Base::customHeaderManipulator* p_manip
					, size_t p_serviceUpdateTime = 0)
					: ios(p_ios)
					, receiver(p_receiver)
					, msgParser(p_msgParser)
					, manip(p_manip)
					, serviceUpdateTime(p_serviceUpdateTime)
				{
				}
			};

			struct acceptorParams{
				std::string addr;
				std::string port;
				size_t workerThreadsCount;
				size_t keepAliveMilliseconds;
				bool usePortReuseChecking;

				acceptorParams(
					const std::string &p_addr
					, const std::string &p_port
					, const size_t p_workerThreadsCount
					, const size_t p_keepAliveMilliseconds
					, const bool p_usePortReuseChecking = false
					)
				{
					addr = p_addr;
					port = p_port;
					workerThreadsCount = p_workerThreadsCount;
					keepAliveMilliseconds = p_keepAliveMilliseconds;
					usePortReuseChecking = p_usePortReuseChecking;
				}
			};

			struct connectorParams{
				std::string addr;
				std::string port;

				connectorParams(
					const std::string &p_addr
					, const std::string &p_port
					)
				{
					addr = p_addr;
					port = p_port;
				}
			};

			virtual ~NetService() {}
			virtual void close(Connection::ptr spConn) = 0;

			virtual boost::asio::io_service& ios() = 0;
			virtual boost::asio::strand& strand() = 0;


			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// acceptor's interface
			static std::shared_ptr<NetServiceAcceptor> createAcceptor(const size_t acceptorIdx
				, serviceInitParams &params
				, acceptorParams &acceptorParams);

			static std::shared_ptr<NetService> getAcceptor(const size_t acceptorIdx);

			virtual void acceptWait(const std::string& addr, const std::string& port
				, size_t workerThreadsCount
				, const size_t keepAliveMilliseconds) = 0;

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// connector's interface
			static std::shared_ptr<NetServiceConnector> createConnector(const size_t connectorIdx
				, serviceInitParams &params
				, connectorParams &connectorParams);

			static std::shared_ptr<NetServiceConnector> getConnector(const size_t connectorIdx);

			virtual void connectWait(const std::string& addr, const std::string& port) = 0;

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// common interface
			virtual void sendPacket(void* sendBuffer, const size_t sendSize) = 0;
			virtual size_t getKeepAliveTimeMs() const = 0;
			static size_t addWorkerThreads(size_t workerCount, boost::asio::io_service& ios);
			static void runService();

		protected:
			static void run(boost::asio::io_service& ios);
			static void initLog();

		protected:
			NetService(EventReceiver& receiver
				, MessageProcedure::customMessageParser msgParser
				, MLN::Base::customHeaderManipulator* manip)
				: MessageProcedure(receiver, msgParser, manip)
			{
				receiver._owner = this;
				receiver.initHandler();
			}
		};
	};//namespace Net
};//namespace MLN
