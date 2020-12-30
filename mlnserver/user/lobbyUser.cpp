#include "stdafx.h"
#include "lobbyUser.h"

#include <packetLobby.h>

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

int User::sendJsonPacket(const std::string& url, Json::Value& jv)
{
	static uint32_t packetSeqNum = 0;

	++packetSeqNum;
	jv[RSP_SEQ] = ++packetSeqNum;

	Json::FastWriter fastWritter;
	std::string serialized = fastWritter.write(jv);

	LOGD("send Json (url:{}) : {}", url, serialized);

	packetLobby::PT_JSON pk;

	// set URL..
	memcpy(pk.url, url.c_str(), url.length());

	//  set Body Size..
	pk.bodySize = (uint16_t)serialized.length();

	//  set Body String..
	memcpy(pk.jsonBody, serialized.c_str(), serialized.length());

	pk.sequenceNo = packetSeqNum;
	pk.isCompressed = 0;

	// send ~!
	return this->Send((char*)&pk, packetLobby::PT_JSON::HEADER_SIZE + pk.bodySize, false);
}