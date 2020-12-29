#pragma once

namespace mln
{
	/*enum class RoomResult
	{
		SUCCESSED = 0,
		FAILED_ROOMKEY = 1,
		ROOM_RESULT_USER_VALUE_BEGIN = 1000
	};*/
	class RoomResultTemplate
	{
	public:
		enum {
			SUCCESSED = 0,
			FAILED_ROOMKEY = 1,

			ROOM_RESULT_USER_VALUE_BEGIN = 1000
		};
	};
}//namespace mln



