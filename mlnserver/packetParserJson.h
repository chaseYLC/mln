#pragma once

#include <net/messageProcedure.h>
#include <net/circularStream.h>
#include <functional>
#include <mutex>
#include <tuple>

#include <packetLobby.h>

namespace mlnserver {

	class PacketJsonParserManip
	{
	public:
		static size_t getHeaderSize()
		{
			return sizeof(packetLobby::HEADER);
		}

		static void facilitate(size_t currentSize, char* buffer)
		{
			packetLobby::HEADER* header = reinterpret_cast<packetLobby::HEADER*>(buffer);
			header->size = currentSize;
			header->code = packetLobby::PT_JSON::packet_value;

			/*packetLobby::PT_JSON *packet = reinterpret_cast<packetLobby::PT_JSON*>(buffer);
			int c = packet->header.size;*/
		}
	};

	class PacketJsonParser
	{
	public:
		static mln::MsgUserManip* getMsgManipulator();

		static bool packetParser(mln::Connection::sptr spConn, mln::CircularStream::Ptr msg
			, mln::MessageProcedure& msg_proc
			, mln::MessageProcedure::msgMapTy& memberFuncMap
			, mln::MessageProcedure::msgMapTy& staticFuncMap);
	};
}//namespace mlnserver {