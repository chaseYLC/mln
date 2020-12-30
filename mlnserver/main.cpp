#include "stdafx.h"

#include <boost/filesystem.hpp>
#include <fmt/core.h>
#include <net/logManager.h>
#include <net/netServiceAcceptor.h>
#include <net/packets/packetParserJson.h>
#include <net/input/inputManager.h>
#include <net/time/time.h>
#include <user/lobbyUser.h>

#include "configuration.h"
#include "keyEventHandler.h"
#include "lobbyAcceptorReceiver.h"

#include "restServer/restServer.h"
#include "watchDogHandler.h"


#ifdef _WIN32
#include <tchar.h>
#include <net/exceptionHandler.h>
#else//#ifdef _WIN32
#include <unistd.h>
#include <limits.h>
#endif//#ifdef _WIN32

#include "clientSample/run.h"


using namespace mlnserver;

void post_asio_start()
{
	LOGI("asio-service started");
	
	RestServer::instance()->Start(
		CONF->GetValueInt(ConfigTags::RESTSVC_BIND_PORT, 28888)
	);

	
	// sample client
	mlnserver::SampleClientTest::TestRun(shared_ioc, CONF->GetValueInt(ConfigTags::SERVER_PORT, 28282));

}

void initTestInterface()
{
	watchDogHandler::instance()->registCallback(
		shared_ioc
		, CONF->GetValueInt(ConfigTags::TELNET_TEST_CONSOLE_PORT, 30000));

	keyEventHandler::instance()->registCallback(shared_ioc);

	mln::InputManager::instance()->start(
		shared_ioc.get()		// use nullptr if not using boost-asio
		//, mln::InputManager::FUNC::wait_keyboardEvent
		, mln::InputManager::FUNC::watchdog
		);
}

bool ioServiceThread()
{
	static LobbyAcceptorReceiver receiver;
	auto lobbyAcceptor = mln::registAcceptor(
		receiver
		, *shared_ioc.get()
		, mln::PacketJsonParser::packetParser
		, mln::PacketJsonParser::getMsgManipulator()
		, CONF->GetValueInt(ConfigTags::UPDATE_TIME_MS, 1000)
		, CONF->GetValueInt(ConfigTags::KEEP_ALIVE, 0) * 1000
		, CONF->GetValueInt(ConfigTags::SERVER_PORT, 28282)
		, CONF->GetValueInt(ConfigTags::NET_IO_WORKER_CNT, boost::thread::hardware_concurrency() * 2)
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

	if (false == CONF->LoadScript("netconfig.json")) {
		fmt::print("Failed LoadScript()");
		return 0;
	}

	// init LogManager
	mln::LogManager::instance()->Create()
		.global()
			.loggerName("mlnserver_log")
			.flushEverySec(CONF->GetValueInt(ConfigTags::LOG_FLUSH_SEC, 0))
		.console()
			.lv(spdlog::level::trace)
			.pattern(nullptr)
		.file()
			.lv(spdlog::level::trace)
			.pattern(nullptr)
			.fileNameBase(CONF->GetValueString(ConfigTags::SERVER_NAME))
			.maxFileSize(1048576 * CONF->GetValueInt(ConfigTags::LOG_FILE_SIZE_MB, 100))
			.maxFiles(CONF->GetValueInt(ConfigTags::LOG_FILE_KEEP_MAX_CNT, 30))
		.done();


#ifdef _WIN32
    mln::ExceptionHandler::Init(nullptr, nullptr, true);

	if (FALSE == SetConsoleTitleA(CONF->GetValueString(ConfigTags::SERVER_NAME).c_str())) {
		fmt::print("SetConsoleTitle failed : {}", GetLastError());
	}
#endif

	initTestInterface();

	return ioServiceThread();
}