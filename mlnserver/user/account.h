#pragma once

#include <cstdint>
#include <set>

namespace mlnserver {

	struct MembershipType
	{
		using Ty = int32_t;
		enum {
			NONE = 0,
		};
	};

	struct AccountType
	{
		using Ty = int32_t;
		enum {
			NONE = 0,
		};
	};

}