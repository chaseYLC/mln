#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

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

		class NetService
		{
		public:
			using PostStartCallbackType = std::function<void()>;

			using AcceptorPtrType = std::shared_ptr<NetServiceAcceptor>;
			using AcceptorContainerType = std::vector< AcceptorPtrType >;

			using ConnectorPtrType = std::shared_ptr<NetServiceConnector>;
			using ConnectorContainerType = std::vector< ConnectorPtrType >;

		public:
			virtual ~NetService() = default;

			virtual boost::asio::io_service& ios() = 0;
			virtual boost::asio::strand& strand() = 0;

		public:
			struct ServiceParams {
				boost::asio::io_service& ios;
				EventReceiver& receiver;
				MessageProcedure::customMessageParser msgParser;
				MsgUserManip* manip;
				size_t serviceUpdateTimeMs;
				size_t keepAliveTimeMs;

				ServiceParams(boost::asio::io_service& p_ios
					, EventReceiver& p_receiver
					, MessageProcedure::customMessageParser p_msgParser
					, MsgUserManip* p_manip
					, size_t p_serviceUpdateTimeMs = 0
					, size_t p_keepAliveTimeMs = 0
					)
					: ios(p_ios)
					, receiver(p_receiver)
					, msgParser(p_msgParser)
					, manip(p_manip)
					, serviceUpdateTimeMs(p_serviceUpdateTimeMs)
					, keepAliveTimeMs(p_keepAliveTimeMs)
				{
				}
			};

			struct AcceptorUserParams {
				std::string addr;
				std::string port;
				size_t workerThreadsCount;
				bool usePortReuseChecking;

				AcceptorUserParams(
					const std::string &p_addr
					, const std::string &p_port
					, const size_t p_workerThreadsCount
					, const bool p_usePortReuseChecking = false
					)
				{
					addr = p_addr;
					port = p_port;
					workerThreadsCount = p_workerThreadsCount;
					usePortReuseChecking = p_usePortReuseChecking;
				}
			};

			struct ConnectorUserParams {
				std::string addr;
				std::string port;

				ConnectorUserParams(
					const std::string &p_addr
					, const std::string &p_port
					)
				{
					addr = p_addr;
					port = p_port;
				}
			};

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// acceptor's interface
			static std::shared_ptr<NetServiceAcceptor> createAcceptor(const size_t acceptorIdx
				, ServiceParams &svcParams
				, AcceptorUserParams &userParams);

			static std::shared_ptr<NetServiceAcceptor> getAcceptor(const size_t acceptorIdx);


			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// connector's interface
			static std::shared_ptr<NetServiceConnector> createConnector(const size_t connectorIdx
				, ServiceParams &svcParams
				, ConnectorUserParams &userParams);

			static std::shared_ptr<NetServiceConnector> getConnector(const size_t connectorIdx);



			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// common interface
			static size_t addWorkerThreads(size_t workerCount);
			static void runService(PostStartCallbackType postServiceStart = nullptr);
			static void runService(boost::asio::io_service& ios, PostStartCallbackType postServiceStart = nullptr);
			size_t getIndex()const { return m_index; }

			static boost::asio::io_service * getCurrentIOS();

		protected:
			NetService() = default;
			static size_t addWorkerThreads(size_t workerCount, boost::asio::io_service& ios);

		private:
			static void run(boost::asio::io_service& ios);
			static void waitServiceStart(boost::asio::io_service* ios, PostStartCallbackType postServiceStart);
			static void setWork();

			void setIndex(size_t idx) { m_index = idx; }

		private:
			size_t m_index;
		};

	};//namespace Net
};//namespace MLN
