#include "stdafx.h"
#include "packetJsonHandler.h"

#include <net/messageProcedure.h>
#include <net/packets/packetJson.h>
#include "clientSample/sampleConnector.h"

using namespace mlnserver;

void JsonPacketHandler::initHandlers(mln::MessageProcedure * msgProc)
{
	JsonPacketHandlerBase::initHandlers(msgProc);

	// lobby
	m_URLs["/lobby/login"] = JsonPacketHandler::login;
}

void JsonPacketHandler::login(mln::Connection::sptr conn, const std::string& url, Json::Value& jv)
{
	GET_USER(conn, spUserBase, user);

	auto myId = std::move(jv["myId"].asString());
	LOGD("received myId:{}", myId);

	Json::Value rsp;
	rsp[RSP_RM] = RSP_OK;
	user->sendJsonPacket(url, rsp);
}