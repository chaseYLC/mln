#include "stdafx.h"
#include "netUtil.h"
#include <boost/tokenizer.hpp>
#include "logManager.h"

namespace mln
{
	bool NetworkUtil::isPrivateAddress(const std::string &ip)
	{
		//* PRIVATE IP
		//- CLASS A : 10.0.0.1 ~ 10.255.255.254
		//- CLASS B : 172.16.0.1 ~ 172.31.255.254
		//- CLASS C : 192.168.0.1 ~ 192.168.255.254
		if (true == ip.empty()) {
			return false;
		}

		using tokenizer = boost::tokenizer<boost::char_separator<char> >;
		tokenizer tok(ip);

		std::vector<std::string> nums;
		for (tokenizer::iterator it = tok.begin(); it != tok.end(); ++it) {
			if (*it == ".") {
				continue;
			}
			nums.push_back(*it);
		}
		if (4 > nums.size()) {
			return false;
		}

		if (nums.at(0) == "169") {// Windows VMWare-------------------------
			return true;
		}
		else if (nums.at(0) == "10") {
			return true;
		}
		else if (nums.at(0) == "172") {
			auto secNum = ::atoi(nums.at(1).c_str());
			if ((secNum >= 16) && (secNum <= 31)) {
				return true;
			}
			return false;
		}
		else if (nums.at(0) == "192") {
			if (nums.at(1) == "168") {
				return true;
			}
			return false;
		}
		return false;
	}

	std::string NetworkUtil::getAcceptorIP_first()
	{
		std::vector<std::string> ips;
		getLocalAddresses(ips);

		if (true == ips.empty()) {
			return "";
		}

		for (auto &ip : ips) {
			if (false == isPrivateAddress(ip.c_str())) {
				return ip;
			}
		}

		return ips.at(ips.size() - 1);
	}

	int NetworkUtil::getLocalAddresses(std::vector<std::string>& ips) {
		#if defined(_WIN32) && defined(NOT_USING_BOOST)
			char sHostname[256] = { 0, };
			char * strIP = 0;
			IN_ADDR inAddr;

			gethostname(sHostname, sizeof(sHostname));
			HOSTENT *hostEnt = gethostbyname(sHostname);

			for (int i = 0; hostEnt->h_addr_list[i] != 0; i++) {
				memcpy(&inAddr, hostEnt->h_addr_list[i], sizeof(IN_ADDR));
				strIP = inet_ntoa(inAddr);

				ips.push_back(strIP);
			}
		#else
			boost::asio::io_context io_context;
			boost::asio::ip::tcp::resolver resolver(io_context);
			boost::asio::ip::tcp::resolver::query query(boost::asio::ip::host_name(),"");
			boost::asio::ip::tcp::resolver::iterator it=resolver.resolve(query);
		
			while(it!=boost::asio::ip::tcp::resolver::iterator())
			{
				boost::asio::ip::address addr=(it++)->endpoint().address();
				if(addr.is_v6()){
					LOGI("ipv6:{}", addr.to_string());
				}
				else{
					LOGI("ipv4:{}", addr.to_string());
				}
				ips.push_back(addr.to_string());
			}
		#endif

		return (int)ips.size();
	}
};//namespace mln
