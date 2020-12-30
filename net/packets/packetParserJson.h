#pragma once

#include <net/messageProcedure.h>
#include <net/circularStream.h>
#include <functional>
#include <mutex>
#include <tuple>

#include "packetJson.h"

namespace mln {

	class PacketJsonParserManip
	{
	public:
		static size_t getHeaderSize()
		{
			return sizeof(packetJson::HEADER);
		}

		static void facilitate(size_t currentSize, char* buffer)
		{
			packetJson::HEADER* header = reinterpret_cast<packetJson::HEADER*>(buffer);
			header->size = currentSize;
			header->code = packetJson::PT_JSON::packet_value;

			/*packetJson::PT_JSON *packet = reinterpret_cast<packetJson::PT_JSON*>(buffer);
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
}//namespace mln {