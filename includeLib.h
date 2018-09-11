#pragma once

#include <Net/netService.h>
#include <Net/connection.h>


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

