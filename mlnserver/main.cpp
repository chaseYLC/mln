#include "stdafx.h"

#include <boost/filesystem.hpp>
#include <fmt/core.h>
#include <net/logManager.h>
#include <net/netServiceAcceptor.h>
#include <net/testInterface.h>
#include <net/time/time.h>
#include <user/lobbyUser.h>

#include "configuration.h"
#include "keyEventHandler.h"
#include "lobbyAcceptorReceiver.h"
#include "packetParserJson.h"
#include "rest_server/restServer.h"
#include "testSample.h"
#include "watchDogHandler.h"


#ifdef _WIN32
#include <tchar.h>
#include <net/exceptionHandler.h>
#else//#ifdef _WIN32
#include <unistd.h>
#include <limits.h>
#endif//#ifdef _WIN32


using namespace mlnserver;

void post_asio_start()
{
	LOGI("asio-service started");
	
	TestFrameworks::Play();

	TRestServer::instance()->Start(
		ConfGetIntD(ConfigTags::RESTSVC_BIND_PORT, 28888)
	);
}

void initTestInterface()
{
	watchDogHandler::instance()->registCallback(
		shared_ios
		, ConfGetIntD(ConfigTags::TELNET_TEST_CONSOLE_PORT, 30000));

	keyEventHandler::instance()->registCallback(shared_ios);

	mln::TestIntarface::instance()->start(
		shared_ios.get()		// use nullptr if not using boost-asio
		//, mln::TestIntarface::FUNC::wait_keyboardEvent
		, mln::TestIntarface::FUNC::watchdog
		);
}

bool ioServiceThread()
{
	static LobbyAcceptorReceiver receiver;
	auto lobbyAcceptor = mln::registAcceptor(
		receiver
		, *shared_ios.get()
		, PacketJsonParser::packetParser
		, PacketJsonParser::getMsgManipulator()
		, ConfGetIntD(ConfigTags::UPDATE_TIME_MS, 1000)
		, ConfGetIntD(ConfigTags::KEEP_ALIVE, 0) * 1000
		, ConfGetIntD(ConfigTags::SERVER_PORT, 28101)
		, ConfGetIntD(ConfigTags::NET_IO_WORKER_CNT, boost::thread::hardware_concurrency() * 2)
	);

	if (!lobbyAcceptor) {
		LOGE("failed acceptor.");
		return false;
	}

	mln::NetService::runService(post_asio_start);

	return true;
}


int main(int argc, char* argv[])
{
	mln::time::initRandom();

	if (false == Configuration::instance()->LoadScript("netconfig.json")) {
		fmt::print("Failed LoadScript()");
		return 0;
	}

	// init LogManager
	mln::LogManager::instance()->Create()
		.global()
			.loggerName("mlnserver_log")
			.flushEverySec(ConfGetIntD(ConfigTags::LOG_FLUSH_SEC, 0))
		.console()
			.lv(spdlog::level::trace)
			.pattern(nullptr)
		.file()
			.lv(spdlog::level::trace)
			.pattern(nullptr)
			.fileNameBase(ConfGetString(ConfigTags::SERVER_NAME))
			.maxFileSize(1048576 * ConfGetIntD(ConfigTags::LOG_FILE_SIZE_MB, 100))
			.maxFiles(ConfGetIntD(ConfigTags::LOG_FILE_KEEP_MAX_CNT, 30))
		.done();


#ifdef USE_DBMS_CONNECTOR_DUMMY
	if (false == DbmsDummy::instance()->LoadScript(
		ConfGetStringD(ConfigTags::DBMS_DUMMY_SCRIPT_NAME, "dbmsdummy_chase.json"))) {
		std::cout << "Failed DBMS-Dummy LoadScript()" << std::endl;
		return 0;
	}
#endif

#ifdef _WIN32
    mln::ExceptionHandler::Init(nullptr, nullptr, true);

	if (FALSE == SetConsoleTitleA(ConfGetString(ConfigTags::SERVER_NAME).c_str())) {
		_tprintf(TEXT("SetConsoleTitle failed (%d)\n"), GetLastError());
	}

#endif

	initTestInterface();

	return ioServiceThread();
}