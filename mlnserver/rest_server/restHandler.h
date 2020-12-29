#pragma once

#include <string>
#include <utility>

namespace mlnserver {
	namespace RestHandler
	{
		enum class ErrorCode
		{
			SUCCESS = 200,
		};
		std::string handler_myrequest(/*rapidjson::Document requestDoc*/);

	}//namespace RestHandler
}