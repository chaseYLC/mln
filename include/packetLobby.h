#pragma once

#include <type_traits>
#include <cstdint>

#pragma warning( push )
#pragma warning( disable : 4351 )


#ifndef CODE_VALUE_MACRO
#define CODE_VALUE_MACRO code = std::decay<decltype(*this)>::type::packet_value
#endif

#pragma pack(1)   

namespace packetLobby
{
	using TyPacketCode = uint32_t;

	struct HEADER {
		uint32_t		size;
		TyPacketCode	code;
	};

	struct PT_JSON
	{
		enum { packet_value = 1 };

		enum {
			MAX_BODY_SIZE = 30000,
			MAX_URL_STRING = 32,
			HEAD_SIZE = 43,
		};

		TyPacketCode	code;
		uint32_t	sequenceNo;
		int8_t		isCompressed = 0;
		int8_t		url[MAX_URL_STRING];
		uint16_t	bodySize = 0;
		int8_t		jsonBody[MAX_BODY_SIZE];

		PT_JSON() {
			CODE_VALUE_MACRO;
			memset(url, 0, sizeof(url));
		}
	};

	struct PT_HEARTBEAT
	{
		enum { packet_value = 3 };

		TyPacketCode	code;

		PT_HEARTBEAT() {
			CODE_VALUE_MACRO;
		}
	};

};// namespace packetLobby




#pragma pack()
#pragma warning ( pop )