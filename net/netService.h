#pragma once

#include <memory>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "eventReceiver.h"
#include "messageProcedure.h"

namespace mln
{
	class CircularStream;
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

		virtual boost::asio::io_context& ios() = 0;
		virtual boost::asio::io_context::strand& strand() = 0;

	public:
		struct ServiceParams {
			boost::asio::io_context& ios;
			EventReceiver& receiver;
			MessageProcedure::customMessageParser msgParser;
			MsgUserManip* manip;
			size_t serviceUpdateTimeMs;
			size_t keepAliveTimeMs;

			ServiceParams(boost::asio::io_context& p_ios
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
				const std::string& p_addr
				, const std::string& p_port
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
				const std::string& p_addr
				, const std::string& p_port
			)
			{
				addr = p_addr;
				port = p_port;
			}
		};

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// acceptor's interface
		static std::shared_ptr<NetServiceAcceptor> createAcceptor(const size_t acceptorIdx
			, ServiceParams& svcParams
			, AcceptorUserParams& userParams);

		static std::shared_ptr<NetServiceAcceptor> createAcceptor(
			ServiceParams& svcParams
			, AcceptorUserParams& userParams);

		static std::shared_ptr<NetServiceAcceptor> getAcceptor(const size_t acceptorIdx);


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// connector's interface
		static std::shared_ptr<NetServiceConnector> createConnector(const size_t connectorIdx
			, ServiceParams& svcParams
			, ConnectorUserParams& userParams);

		static std::shared_ptr<NetServiceConnector> createConnector(
			ServiceParams& svcParams
			, ConnectorUserParams& userParams);

		static std::shared_ptr<NetServiceConnector> getConnector(const size_t connectorIdx);



		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// common interface
		static size_t addWorkerThreads(size_t workerCount);
		static void runService(PostStartCallbackType postServiceStart = nullptr);
		static void runService(boost::asio::io_context& ios, PostStartCallbackType postServiceStart = nullptr);
		size_t getIndex()const { return m_index; }

		static boost::asio::io_context* getCurrentIOS();

	protected:
		NetService() = default;
		static size_t addWorkerThreads(size_t workerCount, boost::asio::io_context& ios);

	private:
		static void run(boost::asio::io_context& ios);
		static void waitServiceStart(boost::asio::io_context* ios, PostStartCallbackType postServiceStart);
		static void setWork();

		void setIndex(size_t idx) { m_index = idx; }

	private:
		size_t m_index;
	};


	template <typename EVENT_RECEIVER_TYPE>
	std::shared_ptr<mln::NetServiceAcceptor> registAcceptor(EVENT_RECEIVER_TYPE &eventReceiver
		, boost::asio::io_context& ioc
		, MessageProcedure::customMessageParser msgParser
		, MsgUserManip* msgManip
		, const size_t serviceUpdateTimeMs
		, const size_t keepAliveTimeMs
		, const uint32_t bindingPort
		, const uint32_t ioWorkerCnt
	)
	{
		mln::EventReceiverAcceptorRegister<EVENT_RECEIVER_TYPE> acceptorHandler(&eventReceiver);

		mln::NetService::ServiceParams serviceInitParams(
			ioc
			, acceptorHandler
			, msgParser
			, msgManip
			, serviceUpdateTimeMs
			, keepAliveTimeMs
		);

		mln::NetService::AcceptorUserParams
			acceptorParams(
				""	// addr. empty string is localhost.
				, std::to_string(bindingPort)
				, ioWorkerCnt != 0 ? ioWorkerCnt : boost::thread::hardware_concurrency() * 2
			);

		auto acceptor = mln::NetService::createAcceptor(
			serviceInitParams
			, acceptorParams
		);

		return acceptor;
	};


};//namespace mln
