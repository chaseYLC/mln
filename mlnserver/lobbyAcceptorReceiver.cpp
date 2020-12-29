#include "stdafx.h"
#include "lobbyAcceptorReceiver.h"

#include <net/logManager.h>
#include <user/lobbyUserManager.h>


void LobbyAcceptorReceiver::onAccept(mln::Connection::sptr spConn)
{
	LOGD("accept - {}/{}"
		, spConn->socket().remote_endpoint().address().to_string()
		, spConn->socket().remote_endpoint().port());

	LobbyUserManager::instance()->addUser(spConn);
}

void LobbyAcceptorReceiver::onClose(mln::Connection::sptr spConn)
{
	LOGD("close - {}/{}"
		, spConn->socket().remote_endpoint().address().to_string()
		, spConn->socket().remote_endpoint().port());

	LobbyUserManager::instance()->closedUser(spConn);
}

void LobbyAcceptorReceiver::onUpdate(uint64_t elapse)
{

}

void LobbyAcceptorReceiver::noHandler(mln::Connection::sptr spConn, mln::CircularStream& msg)
{
	LOGW("no Handler.");
	spConn->closeReserve(0);
}

void LobbyAcceptorReceiver::onAcceptFailed(mln::Connection::sptr spConn)
{
	LOGW("failed accept");
}
void LobbyAcceptorReceiver::onCloseFailed(mln::Connection::sptr spConn)
{
	LOGW("failed close");
}

void LobbyAcceptorReceiver::onExpiredSession(mln::Connection::sptr spConn)
{
	auto endPoint = spConn->socket().remote_endpoint();
	LOGW("Expired Session. (addr/port) : {}/{}"
		, endPoint.address().to_string(), endPoint.port());

	 spConn->closeReserve(0);
}

void LobbyAcceptorReceiver::initHandler(mln::MessageProcedure *msgProc)
{
}

