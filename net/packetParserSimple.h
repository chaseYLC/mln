#pragma once

#include "messageProcedure.h"
#include "messageBuffer.h"
#include <functional>
#include <mutex>
#include <tuple>

#define SIMPLE_PARSER_MAIN_KEY 0xffff0000
#define SIMPLE_PARSER_PART_KEY 0xfff30000

#define SIMPLE_PARSER_KEY_MAKEKEY(key, len)	((key & 0xffff0000) | (len & 0x0000ffff))
#define SIMPLE_PARSER_KEY_GETKEY(key)			(key & 0xffff0000)
#define SIMPLE_PARSER_KEY_GETLENGTH(key)		(key & 0x0000ffff)

struct SimpleParserProposal
{
	enum{
		MAX_PACKET = 2048,
		PARTKEY_SIZE = 4,
		HEADER_SIZE = PARTKEY_SIZE + PARTKEY_SIZE,
	};
};

class message_header_manipulator_simpleParser
{
public:
	struct HEADER
	{
		int32_t mainKey;
		int32_t partKey;
	};

	static size_t getHeaderSize()
	{
		return sizeof(HEADER);
	}

	static void facilitate(size_t currentSize, char *buffer)
	{
		const int32_t  bodySize = (int32_t)(currentSize - sizeof(HEADER));

		((HEADER*)buffer)->mainKey = SIMPLE_PARSER_KEY_MAKEKEY(SIMPLE_PARSER_MAIN_KEY, SimpleParserProposal::PARTKEY_SIZE + bodySize);
		((HEADER*)buffer)->partKey = SIMPLE_PARSER_KEY_MAKEKEY(SIMPLE_PARSER_PART_KEY, bodySize);
	}
};

class SIMPLE_PACKET_PARSING_SUPPORT
{
public:
	static MLN::Net::MsgUserManip* getMsgManipulator();

	static bool packetParser(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
		, MLN::Net::MessageProcedure &msg_proc
		, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
		, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap);

	static bool packetParser_notUseEnc(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
		, MLN::Net::MessageProcedure &msg_proc
		, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
		, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap);

	static bool packetParser_performanced(MLN::Net::Connection::ptr spConn, MLN::Net::MessageBuffer::Ptr msg
		, MLN::Net::MessageProcedure &msg_proc
		, MLN::Net::MessageProcedure::msgMapTy& memberFuncMap
		, MLN::Net::MessageProcedure::msgMapTy& staticFuncMap);
};
