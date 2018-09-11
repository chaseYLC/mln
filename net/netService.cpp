#include "stdafx.h"

#include "logger.h"
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
		static boost::asio::io_service * s_currentIOS = nullptr;
		static boost::asio::deadline_timer * s_insuranceObjForStart = nullptr;
		static boost::asio::io_service::work	* s_work = nullptr;

		void NetService::runService(PostStartCallbackType postServiceStart /*= nullptr*/)
		{
			bool initializedService = false;

			size_t idxAnyService = 0;
			for (; idxAnyService < s_acceptors.size(); ++idxAnyService) {
				if (nullptr != s_acceptors[idxAnyService]) {
					initializedService = true;
					s_currentIOS = &NetService::getAcceptor(idxAnyService)->ios();
					break;
				}
			}
			if (initializedService == false) {
				idxAnyService = 0;
				for (; idxAnyService < s_connectors.size(); ++idxAnyService) {
					if (nullptr != s_connectors[idxAnyService]) {
						initializedService = true;
						s_currentIOS = &NetService::getConnector(idxAnyService)->ios();
						break;
					}
				}
			}

			assert(s_currentIOS && "set ios first");

			setWork();

			if (true == s_threads.empty()) {
				boost::shared_ptr<boost::thread> thread(new boost::thread(
					boost::bind(&boost::asio::io_service::run, s_currentIOS)));

				s_threads.push_back(thread);
			}

			if (nullptr != postServiceStart) {
				boost::thread watThread = boost::thread(&NetService::waitServiceStart, s_currentIOS, postServiceStart);
				run(*s_currentIOS);
			}
			else {
				run(*s_currentIOS);
			}
		}

		void NetService::runService(boost::asio::io_service& ios, PostStartCallbackType postServiceStart /*= nullptr*/)
		{
			if (nullptr == s_currentIOS) {
				s_currentIOS = &ios;
			}

			setWork();

			if (true == s_threads.empty()) {
				boost::shared_ptr<boost::thread> thread(new boost::thread(
					boost::bind(&boost::asio::io_service::run, &ios)));

				s_threads.push_back(thread);
			}

			if (nullptr != postServiceStart) {
				boost::thread watThread = boost::thread(boost::bind(&NetService::waitServiceStart, &ios, postServiceStart));
				run(ios);
			}
			else {
				run(ios);
			}

		}

		void NetService::waitServiceStart(boost::asio::io_service* ios, PostStartCallbackType postServiceStart)
		{
			while (true) {
				if (false == ios->stopped()) {
					break;
				}

				boost::this_thread::sleep(boost::posix_time::milliseconds(30));
			}

			postServiceStart();
		}

		std::shared_ptr<NetServiceAcceptor> NetService::createAcceptor(const size_t acceptorIdx
			, ServiceParams &svcParams
			, AcceptorUserParams &userParams)
		{
			if (s_acceptors.size() <= acceptorIdx) {
				s_acceptors.resize(acceptorIdx + 1);
			}

			if (nullptr == s_acceptors[acceptorIdx])
			{
				s_acceptors[acceptorIdx] = std::make_shared<NetServiceAcceptor>(svcParams);
				s_acceptors[acceptorIdx]->setIndex(acceptorIdx);

				// port reuse checking.
				/*if (true == userParams.usePortReuseChecking) {
				}*/

				// accept
				s_acceptors[acceptorIdx]->acceptWait(
					userParams.addr
					, userParams.port
					, userParams.workerThreadsCount);

			}
			return s_acceptors[acceptorIdx];
		}

		std::shared_ptr<NetServiceAcceptor> NetService::getAcceptor(const size_t acceptorIdx)
		{
			assert(s_acceptors.size() > acceptorIdx);
			if (s_acceptors.size() <= acceptorIdx) {
				return nullptr;
			}

			return s_acceptors[acceptorIdx];
		}

		std::shared_ptr<NetServiceConnector> NetService::createConnector(const size_t connectorIdx
			, ServiceParams &svcParams
			, ConnectorUserParams &userParams)
		{
			if (s_connectors.size() <= connectorIdx) {
				s_connectors.resize(connectorIdx + 1);
			}

			if (nullptr == s_connectors[connectorIdx])
			{
				s_connectors[connectorIdx] = std::make_shared<NetServiceConnector>(svcParams, userParams);
				s_connectors[connectorIdx]->setIndex(connectorIdx);
			}

			return s_connectors[connectorIdx];
		}

		std::shared_ptr<NetServiceConnector> NetService::getConnector(const size_t connectorIdx)
		{
			assert(s_connectors.size() > connectorIdx);
			if (s_connectors.size() <= connectorIdx) {
				return nullptr;
			}

			return s_connectors[connectorIdx];
		}

		void NetService::setWork()
		{
			s_work = new boost::asio::io_service::work(*s_currentIOS);
			s_insuranceObjForStart = new boost::asio::deadline_timer(*s_currentIOS);
			s_insuranceObjForStart->expires_from_now(
				boost::posix_time::hours(1));

			s_insuranceObjForStart->async_wait([](const boost::system::error_code&) {});
		}

		void NetService::run(boost::asio::io_service& ios)
		{
			s_currentIOS = &ios;

			LOGI << "MLN-Net run..";

			ios.run();

			for (auto it : s_threads) {
				it->join();
			}
			s_threads.clear();

			LOGI << "MLN-Net stop..";
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

		size_t NetService::addWorkerThreads(size_t workerCount)
		{
			if (nullptr == s_currentIOS) {
				LOGE << "set IOS first.";
				return 0;
			}

			for (size_t i = 0; i < workerCount; ++i)
			{
				boost::shared_ptr<boost::thread> thread(new boost::thread(
					boost::bind(&boost::asio::io_service::run, s_currentIOS)));

				s_threads.push_back(thread);
			}

			return s_threads.size();
		}

		boost::asio::io_service * NetService::getCurrentIOS()
		{
			return s_currentIOS;
		}

	};//namespace Net
};//namespace MLN