#pragma once

#include <stdint.h>

namespace MLN
{
	namespace Net
	{
		struct EncType
		{
			using Type = uint8_t;

			enum {
				GREETING = 0,

				AES_128_CBC = 2,
				AES_128_CBC_keR = 3,
			};

		};
	}//namespace Net
};//namespace MLN