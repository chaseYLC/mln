#include "stdafx.h"

#include <Base/Base/include/log.h>
#include <Base/Base/include/runningAlone.h>
#include "ConnectionImpl.h"
#include "EventReceiver.h"
#include "netServiceAcceptor.h"
#include "netServiceConnector.h"


namespace MLN
{
	namespace Net
	{
		static NetService::AcceptorContainerType s_acceptors;
		static NetService::ConnectorContainerType s_connectors;

		static std::vector<boost::shared_ptr<boost::thread>> s_threads;

		void NetService::runService()
		{
			bool initializedService = false;
			boost::asio::io_service *this_ios = nullptr;

			size_t idxAnyService = 0;
			for (; idxAnyService < s_acceptors.size(); ++idxAnyService){
				if (nullptr != s_acceptors[idxAnyService]){
					initializedService = true;
					this_ios = &NetService::getAcceptor(idxAnyService)->ios();
					break;
				}
			}
			if (initializedService == false){
				idxAnyService = 0;
				for (; idxAnyService < s_connectors.size(); ++idxAnyService){
					if (nullptr != s_connectors[idxAnyService]){
						initializedService = true;
						this_ios = &NetService::getConnector(idxAnyService)->ios();
						break;
					}
				}
			}

			assert(true == initializedService
				&& "service not initialized");

			if (true == s_threads.empty()){
				boost::shared_ptr<boost::thread> thread(new boost::thread(
					boost::bind(&boost::asio::io_service::run, this_ios)));

				s_threads.push_back(thread);
			}

			run(*this_ios);
		}

		std::shared_ptr<NetServiceAcceptor> NetService::createAcceptor(const size_t acceptorIdx
			, serviceInitParams &params
			, acceptorParams &acceptorParams)
		{
			if (s_acceptors.size() <= acceptorIdx) {
				s_acceptors.reserve(acceptorIdx + 1);
				s_acceptors.resize(acceptorIdx + 1);
			}

			if (nullptr == s_acceptors[acceptorIdx])
			{
				s_acceptors[acceptorIdx] = std::make_shared<NetServiceAcceptor>(params);

				// port reuse checking.
				if (true == acceptorParams.usePortReuseChecking){
					MLN::Base::RunningAlone *runningAlone
						= s_acceptors[acceptorIdx]->getRunningAloneCheckObject();

					if (MLN::Base::RunningAlone::RetCode::OK != runningAlone->init(
						acceptorParams.port
						, MLN::Base::RunningAlone::StoreType::file)){
						LOGE << "TCP Port is duplicating. Port Number : " << acceptorParams.port;

						return nullptr;
					}
				}

				// accept
				s_acceptors[acceptorIdx]->acceptWait(
					acceptorParams.addr
					, acceptorParams.port
					, acceptorParams.workerThreadsCount
					, acceptorParams.keepAliveMilliseconds);
			}
			return s_acceptors[acceptorIdx];
		}

		std::shared_ptr<NetService> NetService::getAcceptor(const size_t acceptorIdx)
		{
			assert(s_acceptors.size() > acceptorIdx);
			if (s_acceptors.size() <= acceptorIdx){
				return nullptr;
			}

			return s_acceptors[acceptorIdx];
		}

		std::shared_ptr<NetServiceConnector> NetService::createConnector(const size_t connectorIdx
			, serviceInitParams &params
			, connectorParams &connectorParams)
		{
			if (s_connectors.size() <= connectorIdx) {
				s_connectors.reserve(connectorIdx + 1);
				s_connectors.resize(connectorIdx + 1);
			}

			if (nullptr == s_connectors[connectorIdx])
			{
				s_connectors[connectorIdx] = std::make_shared<NetServiceConnector>(params);
			}

			return s_connectors[connectorIdx];
		}

		std::shared_ptr<NetServiceConnector> NetService::getConnector(const size_t connectorIdx)
		{
			assert(s_connectors.size() > connectorIdx);
			if (s_connectors.size() <= connectorIdx){
				return nullptr;
			}

			return s_connectors[connectorIdx];
		}

		void NetService::run(boost::asio::io_service& ios)
		{
			LOGI << "MLNNet run..";

			ios.run();

			for (auto it : s_threads){
				it->join();
			}
			s_threads.clear();

			LOGI << "MLNNet stop..";
	}

		void NetService::initLog()
		{
#ifdef USE_INTERNAL_BOOST_LOG_MLNNET
			using namespace boost::log;

			core::get()->add_global_attribute("TimeStamp", attributes::utc_clock());
			core::get()->add_global_attribute("Scope", attributes::named_scope());

			core::get()->set_filter(trivial::severity >= trivial::info);

			/* log formatter:
			* [TimeStamp] [ThreadId] [Severity Level] [Scope] Log message
			*/
			auto fmtTimeStamp = expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");
			auto fmtThreadId = expressions::
				attr<attributes::current_thread_id::value_type>("ThreadID");
			auto fmtSeverity = expressions::
				attr<trivial::severity_level>("Severity");
			auto fmtScope = expressions::format_named_scope("Scope",
				keywords::format = "%n(%f:%l)",
				keywords::iteration = expressions::reverse,
				keywords::depth = 2);
			formatter logFmt =
				expressions::format("[%1%] (%2%) [%3%] [%4%] %5%")
				% fmtTimeStamp % fmtThreadId % fmtSeverity % fmtScope
				% expressions::smessage;

			/* console sink */
			auto consoleSink = add_console_log(std::clog);
			consoleSink->set_formatter(logFmt);

			/* fs sink */
			auto fsSink = add_file_log(
				keywords::file_name = "logs/test_%Y-%m-%d_%H-%M-%S.%N.log",
				keywords::rotation_size = 10 * 1024 * 1024, //10mb마다 rotate
				keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), //12시마다 rotate
				keywords::min_free_space = 30 * 1024 * 1024,
				keywords::auto_flush = true,
				keywords::open_mode = std::ios_base::app);
			fsSink->set_formatter(logFmt);
			/*fsSink->locked_backend()->auto_flush(true);*/
#endif
}

		size_t NetService::addWorkerThreads(size_t workerCount, boost::asio::io_service& ios)
		{
			for (size_t i = 0; i < workerCount; ++i)
			{
				boost::shared_ptr<boost::thread> thread(new boost::thread(
					boost::bind(&boost::asio::io_service::run, &ios)));

				s_threads.push_back(thread);
			}

			return s_threads.size();
		}

};//namespace Net


};//namespace MLN