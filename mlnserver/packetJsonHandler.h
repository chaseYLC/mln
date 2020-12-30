#pragma once

#include <net/packets/packetJsonHandlerBase.h>

namespace mlnserver {

	class JsonPacketHandler
		: public mln::JsonPacketHandlerBase
	{
	public:
		static void initHandlers(mln::MessageProcedure* msgProc);

	public:
		static void login(mln::Connection::sptr conn, const std::string& url, Json::Value& jv);
	};
}//namespace mlnserver {