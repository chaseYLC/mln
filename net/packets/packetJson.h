#pragma once

#include <type_traits>
#include <cassert>
#include <cstdint>

#pragma warning( push )
#pragma warning( disable : 4351 )


#ifndef PACKET_LOBBY_CODE_VALUE_MACRO
#define PACKET_LOBBY_CODE_VALUE_MACRO header.code = std::decay<decltype(*this)>::type::packet_value
#endif

#pragma pack(1)   

namespace packetJson
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
			MAX_BODY_SIZE = 8192,
			MAX_URL_STRING = 32,
			HEADER_SIZE = 47,
		};

#pragma region JsonPacketHeader
		HEADER		header;
		uint32_t	sequenceNo;
		int8_t		isCompressed = 0;
		int8_t		url[MAX_URL_STRING];
		uint16_t	bodySize = 0;
#pragma endregion
		int8_t		jsonBody[MAX_BODY_SIZE];

		PT_JSON() {
			static_assert(HEADER_SIZE == sizeof(PT_JSON) - sizeof(jsonBody)
				, "check header-size");

			PACKET_LOBBY_CODE_VALUE_MACRO;
			memset(url, 0, sizeof(url));
		}
	};

	struct PT_HEARTBEAT
	{
		enum { packet_value = 3 };

		HEADER		header;

		PT_HEARTBEAT() {
			PACKET_LOBBY_CODE_VALUE_MACRO;
		}
	};

};// namespace packetJson




#pragma pack()
#pragma warning ( pop )