#include "stdafx.h"
#include "lobbyUser.h"

using namespace mlnserver;

std::string User::GetForceClosingInfo()
{
	if (auto spConn = _conn.lock(); spConn) {
		return fmt::format(
			"userIDX:{}\
, ip:{}\
"
, m_userIDX
, spConn->socket().remote_endpoint().address().to_string()
);
	}
	else {
		return "loss session";
	}
}

std::string User::GetUserID() const
{
	return std::to_string(m_userIDX);
}
