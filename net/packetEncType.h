#pragma once

#include <stdint.h>

namespace mln
{
	struct EncType
	{
		using Type = uint8_t;

		enum {
			GREETING = 0,

			AES_128_CBC = 2,
			AES_128_CBC_keR = 3,
			KPP = 11,
		};

	};
};//namespace mln