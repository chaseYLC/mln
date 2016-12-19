#pragma once

#include <CFCNet/net/include/CFCNet_service.h>
#include <CFCNet/net/include/connection.h>


#ifdef _WIN64

#ifdef _DEBUG
#pragma comment(lib, "CFCNet/lib64/CFCNet_x64D.lib")
#else
#pragma comment(lib, "CFCNet/lib64/CFCNet_x64.lib")
#endif

#elif _WIN32

#ifdef _DEBUG
#pragma comment(lib, "CFCNet/lib/CFCNetD.lib")
#else
#pragma comment(lib, "CFCNet/lib/CFCNet.lib")
#endif

#endif

