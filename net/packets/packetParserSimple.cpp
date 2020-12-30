#include "stdafx.h"
#include "packetParserSimple.h"
#include <net/packetEncType.h>
#include <net/eventReceiver.h>

namespace mln {

	static std::once_flag message_header_simple_packet_manipulator_flag;

	mln::MsgUserManip* SIMPLE_PACKET_PARSING_SUPPORT::getMsgManipulator()
	{
		static mln::MsgUserManip msgMainp;

		std::call_once(message_header_simple_packet_manipulator_flag
			, [&] {

				msgMainp = std::make_tuple(
					message_header_manipulator_simpleParser::getHeaderSize
					, message_header_manipulator_simpleParser::facilitate
				);
			});

		return &msgMainp;
	}

	bool SIMPLE_PACKET_PARSING_SUPPORT::packetParser(mln::Connection::sptr spConn, mln::CircularStream::Ptr msg
		, mln::MessageProcedure& msg_proc
		, mln::MessageProcedure::msgMapTy& memberFuncMap
		, mln::MessageProcedure::msgMapTy& staticFuncMap)
	{
		using namespace mln;

		static const uint8_t defaultCipherType = mln::EncType::AES_128_CBC;

		bool readStartFromMainKey = true;

		message_header_manipulator_simpleParser::HEADER header;
		const size_t sizeHeader = sizeof(header);

		uint32_t total_size = 0;

		do {
			if (true == readStartFromMainKey) {
				if (false == msg->readable((char*)&header, sizeHeader)) {
					break;
				}

				// mainKey check.
				if (SIMPLE_PARSER_KEY_GETKEY(header.mainKey) != 0xffff0000) {
					msg->readAll();
					return false;
				}

				total_size = SIMPLE_PARSER_KEY_GETLENGTH(header.mainKey) + sizeof(header.mainKey);
				if (false == msg->readable(total_size)) {
					break;
				}
				msg->read(sizeof(header.mainKey));
			}
			else {

			}

			msg->read((char*)&header.partKey, sizeof(header.partKey));
			// partKey Check
			if (SIMPLE_PARSER_KEY_GETKEY(header.partKey) != 0xfff30000)
			{
				// err
				msg->readAll();
				return false;
			}
			uint32_t part_body_size = SIMPLE_PARSER_KEY_GETLENGTH(header.partKey);
			uint32_t part_size = part_body_size + sizeof(header.partKey);
			if (false == msg->readable(part_body_size)) {
				// err
				msg->readAll();
				return false;
			}

			total_size -= part_size;

			readStartFromMainKey = (total_size == sizeof(header.partKey));

			CircularStream procMsg;

			auto ciperData = (MessageProcedure::NOT_SET_CIPHER_VALUE == msg_proc.getUserCipherType())
				? msg_proc.getCipherHandler(defaultCipherType)
				: msg_proc.getCipherHandler((uint8_t)msg_proc.getUserCipherType());

			if (std::get<1>(ciperData))
			{
				auto ciperType = std::get<0>(ciperData);
				auto cipherInitHandler = std::get<1>(ciperData);
				auto cipherDecHandler = std::get<2>(ciperData);

				if (false == spConn->isSyncEncryptionKey(ciperType)) {
					if (false == cipherInitHandler(ciperType, nullptr, spConn, part_body_size, *msg.get())) {
						msg->readAll();
						return false;
					}

					if (false == msg->readable(part_body_size)) {
						msg->readAll();
						return false;
					}
					msg->read(part_body_size);
					continue;
				}
				else {
					if (mln::EncType::GREETING == ciperType) {
						msg->read(procMsg.enableBuffer(), part_body_size);
						procMsg.write(part_body_size);
					}
					else {
						assert(NULL != cipherDecHandler);
						cipherDecHandler(ciperType, spConn, part_body_size, *msg.get(), procMsg);
						msg->read(part_body_size);

						if (0 >= procMsg.size()) {
							break;
						}
					}
				}
			}//if (nullptr != std::get<1>(ciperData))
			else {
				msg->read(procMsg.enableBuffer(), part_body_size);
				procMsg.write(part_body_size);
			}

			int32_t packetCode = 0;
			procMsg.readable((char*)&packetCode, sizeof(packetCode));

			auto it = memberFuncMap.find(packetCode);
			if (memberFuncMap.end() != it) {
				it->second(spConn, part_body_size, procMsg);
			}
			else {
				auto it = staticFuncMap.find(packetCode);
				if (staticFuncMap.end() != it) {
					it->second(spConn, part_body_size, procMsg);
				}
				else {
					spConn->getReceiver()->noHandler(spConn, procMsg);
					msg->readAll();
					return false;
				}
			}

		} while (true);

		return true;
	}

	bool SIMPLE_PACKET_PARSING_SUPPORT::packetParser_notUseEnc(mln::Connection::sptr spConn, mln::CircularStream::Ptr msg
		, mln::MessageProcedure& msg_proc
		, mln::MessageProcedure::msgMapTy& memberFuncMap
		, mln::MessageProcedure::msgMapTy& staticFuncMap)
	{
		using namespace mln;
		bool readStartFromMainKey = true;

		message_header_manipulator_simpleParser::HEADER header;
		const size_t sizeHeader = sizeof(header);

		uint32_t total_size = 0;

		do {
			if (true == readStartFromMainKey) {
				if (false == msg->readable((char*)&header, sizeHeader)) {
					break;
				}

				// mainKey check.
				if (SIMPLE_PARSER_KEY_GETKEY(header.mainKey) != 0xffff0000) {
					/*LOG_SERVICE_ERR("instance", "Session[%d] %s - %u - current = %X", ident, __FUNCTION__, __LINE__, recv_buffer.GetCursor());*/
					msg->readAll();
					return false;
				}

				total_size = SIMPLE_PARSER_KEY_GETLENGTH(header.mainKey) + sizeof(header.mainKey);
				if (false == msg->readable(total_size)) {
					break;
				}
				msg->read(sizeof(header.mainKey));
			}
			else {

			}

			msg->read((char*)&header.partKey, sizeof(header.partKey));
			// partKey Check
			if (SIMPLE_PARSER_KEY_GETKEY(header.partKey) != 0xfff30000)
			{
				// err
				msg->readAll();
				return false;
			}
			uint32_t part_body_size = SIMPLE_PARSER_KEY_GETLENGTH(header.partKey);
			uint32_t part_size = part_body_size + sizeof(header.partKey);
			if (false == msg->readable(part_body_size)) {
				// err
				msg->readAll();
				return false;
			}

			total_size -= part_size;

			readStartFromMainKey = (total_size == sizeof(header.partKey));

			mln::CircularStream procMsg;
			msg->read(procMsg.enableBuffer(), part_body_size);
			procMsg.write(part_body_size);

			int32_t packetCode = 0;
			procMsg.readable((char*)&packetCode, sizeof(packetCode));

			auto it = memberFuncMap.find(packetCode);
			if (memberFuncMap.end() != it) {
				it->second(spConn, part_body_size, procMsg);
			}
			else {
				auto it = staticFuncMap.find(packetCode);
				if (staticFuncMap.end() != it) {
					it->second(spConn, part_body_size, procMsg);
				}
				else {
					spConn->getReceiver()->noHandler(spConn, procMsg);
					msg->readAll();
					return false;
				}
			}

		} while (true);

		return true;
	}


	bool SIMPLE_PACKET_PARSING_SUPPORT::packetParser_performanced(mln::Connection::sptr spConn, mln::CircularStream::Ptr msg
		, mln::MessageProcedure& msg_proc
		, mln::MessageProcedure::msgMapTy& memberFuncMap
		, mln::MessageProcedure::msgMapTy& staticFuncMap)
	{
		return false;

	}
}//namespace mln {