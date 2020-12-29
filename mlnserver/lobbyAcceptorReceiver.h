#pragma once

#include <net/connection.h>
#include <net/messageProcedure.h>

class LobbyAcceptorReceiver
{
	//////////////////////////////////////////////////////////////////
public:
	void onAccept(mln::Connection::sptr spConn);
	void onAcceptFailed(mln::Connection::sptr spConn);

	void onClose(mln::Connection::sptr spConn);
	void onCloseFailed(mln::Connection::sptr spConn);

	void onUpdate(uint64_t elapse);

	void onExpiredSession(mln::Connection::sptr spConn);

	void noHandler(mln::Connection::sptr spConn, mln::CircularStream& msg);
	//////////////////////////////////////////////////////////////////
public:
	void initHandler(mln::MessageProcedure* msgProc);
};