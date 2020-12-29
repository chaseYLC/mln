#pragma once

#pragma warning(disable: 4819)	// for spdlog-circular_q.h
#pragma warning(disable: 4312)	// for lockfreeLinkedlist.h


#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_UNWRAP
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/pool/pool.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>

#include <net/logManager.h>



inline std::shared_ptr< boost::asio::io_context > shared_ios
 = std::make_shared<boost::asio::io_context>();

inline std::mt19937 *g_rand;

typedef struct tagTSV_ID {
	enum {
		DB_ERROR = 1,

		REDIS_CLIENT = 2,
	};
}TSV_ID;

#define RAND_NO		((*g_rand)())


#define RSP_SEQ	"packetSequenceNum"
#define RSP_RC	"resultCode"
#define RSP_RM	"resultMsg"
#define RSP_OK	"OK"
#define RSP_RC_SYSTEM_ERROR		99


#ifndef MEMO
#define chSTR2(x) #x
#define chSTR(x)  chSTR2(x)
#define MEMO(desc) message(__FILE__ "(" chSTR(__LINE__) "):" #desc)
#endif

