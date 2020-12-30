#include "stdafx.h"
#include "packetParserJson.h"

#include <net/packetEncType.h>
#include <net/eventReceiver.h>
#include "packetJson.h"


namespace mln {

	static std::once_flag s_jsonParserManipFlag;

	mln::MsgUserManip* PacketJsonParser::getMsgManipulator()
	{
		static mln::MsgUserManip msgMainp;

		std::call_once(s_jsonParserManipFlag
			, [&] {

				msgMainp = std::make_tuple(
					PacketJsonParserManip::getHeaderSize
					, PacketJsonParserManip::facilitate
				);
			});

		return &msgMainp;
	}

	bool PacketJsonParser::packetParser(mln::Connection::sptr spConn, mln::CircularStream::Ptr msg
		, mln::MessageProcedure& msg_proc
		, [[maybe_unused]] mln::MessageProcedure::msgMapTy& memberFuncMap
		, mln::MessageProcedure::msgMapTy& staticFuncMap)
	{
		packetJson::HEADER header;

		do {
			if (false == msg->readable((char*)&header, sizeof(header))) {
				break;
			}

			if (header.size < 0 || header.size > USHRT_MAX) {
				return false;
			}

			if (false == msg->readable(header.size)) {
				break;
			}

			auto packet = std::make_shared<mln::CircularStream>();
			packet->write((char*)(msg->data()), header.size);
			msg->read(header.size);

			if (auto it = staticFuncMap.find(header.code); staticFuncMap.end() != it) {
				it->second(spConn, header.size, *packet.get());
			}
			else {
				spConn->getReceiver()->noHandler(spConn, *packet.get());
				msg->readAll();
				return false;
			}

		} while (true);

		return true;
	}
}//namespace mln {