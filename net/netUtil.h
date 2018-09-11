#pragma once

#include <string>
#include <vector>

namespace MLN
{
	namespace Net
	{
		class NetworkUtil
		{
		public:
			static bool isPrivateAddress(const std::string &ip);
			static std::string getAcceptorIP_first();
			static int getLocalAddresses(std::vector<std::string>& ips);

		};
	}
}
