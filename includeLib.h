#pragma once

#include <net/netService.h>
#include <net/connection.h>


#ifdef _WIN64

#ifdef _DEBUG
#pragma comment(lib, "lib64/net_x64D.lib")
#else
#pragma comment(lib, "lib64/net_x64.lib")
#endif

#elif _WIN32

#ifdef _DEBUG
#pragma comment(lib, "MLN/Net/lib/NetD.lib")
#else
#pragma comment(lib, "MLN/Net/lib/Net.lib")
#endif

#endif

