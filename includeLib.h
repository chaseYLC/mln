#pragma once

#include <Net/net/include/netService.h>
#include <Net/net/include/connection.h>


#ifdef _WIN64

#ifdef _DEBUG
#pragma comment(lib, "Net/lib64/Net_x64D.lib")
#else
#pragma comment(lib, "Net/lib64/Net_x64.lib")
#endif

#elif _WIN32

#ifdef _DEBUG
#pragma comment(lib, "Net/lib/NetD.lib")
#else
#pragma comment(lib, "Net/lib/Net.lib")
#endif

#endif

