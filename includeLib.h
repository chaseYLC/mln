#pragma once

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

#include <array>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <stdint.h>
#include <string>
#include <tchar.h>
#include <vector>

#include <net/macros.h>
#include <net/logger.h>
#include <net/netService.h>
#include <net/connection.h>


#ifdef _WIN64

#ifdef _DEBUG
#pragma comment(lib, "MLN/Net/lib64/Net_x64D.lib")
#else
#pragma comment(lib, "MLN/Net/lib64/Net_x64.lib")
#endif

#elif _WIN32

#ifdef _DEBUG
#pragma comment(lib, "MLN/Net/lib/NetD.lib")
#else
#pragma comment(lib, "MLN/Net/lib/Net.lib")
#endif

#endif

