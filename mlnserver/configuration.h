#pragma once 

#include <map>
#include <variant>
#include <net/enumStrings.h>
#include <net/config/config.h>

ENUM_WITH_STRING_CONVERSION(ConfigTags,
	(ENV)
	(version)
	(SERVER_NAME)
	(SERVER_PORT)
	(KEEP_ALIVE)
	(UPDATE_TIME_MS)
	(UPDATE_TIME_FOR_ROOM_MS)
	(UPDATE_TIME_FOR_MONITOR_MS)
	(LOG_FLUSH_SEC)
	(LOG_FILE_SIZE_MB)
	(LOG_FILE_KEEP_MAX_CNT)
	(HTTP_REQUEST_TIMEOUT)
	(RESTSVC_BIND_PORT)
	(NET_IO_WORKER_CNT)

	(TELNET_TEST_CONSOLE_PORT)
)

#define CONF	mln::Config<ConfigTags>::instance()