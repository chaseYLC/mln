#pragma once

#include <net/connection.h>
#include <net/json/json.h>
#include <functional>

namespace mln {
	class MessageProcedure;
}

namespace mlnserver {

	class JsonPacketHandler
	{
	public:
		static void initHandlers(mln::MessageProcedure* msgProc);
		static bool readJsonPacket(mln::Connection::sptr conn, unsigned int size, mln::CircularStream& msg);
		static void dispatch(mln::Connection::sptr conn, const std::string& url, Json::Value& jv);

	public:
		static void login(mln::Connection::sptr conn, const std::string& url, Json::Value& jv);

	private:
		inline static std::map<std::string
			, std::function< void(mln::Connection::sptr, const std::string&, Json::Value&) > > m_URLs;
	};
}//namespace mlnserver {