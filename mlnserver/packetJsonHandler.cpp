#include "stdafx.h"
#include "packetJsonHandler.h"

#include <net/messageProcedure.h>
#include <packetLobby.h>
#include "clientSample/sampleConnector.h"

using namespace mlnserver;

#define GET_USER_LOBBY(user)	mlnserver::User *user = (mlnserver::User *)conn->getUser();	if (nullptr == user){	LOGE("user is null. ident: {}", conn->getIdentity());	return;}
#define GET_USER_LOBBY_BY_CONN(user, conn)	mlnserver::User *user = (mlnserver::User *)conn->getUser();	if (nullptr == user){	LOGE("user is null. ident: {}", conn->getIdentity());	return;}

#define GET_USER(conn, spUserBase, user)	\
	std::shared_ptr<mln::UserBasis> spUserBase = conn->getUser();\
	if (!spUserBase) {\
		LOGE("none user. sessionId:{}", conn->getIdentity());\
		return;\
	}\
	std::shared_ptr<mlnserver::SampleUser> user = std::static_pointer_cast<mlnserver::SampleUser>(spUserBase)



void JsonPacketHandler::initHandlers(mln::MessageProcedure * msgProc)
{
	msgProc->registMessage(packetLobby::PT_JSON::packet_value, &JsonPacketHandler::readJsonPacket);
	
	// lobby
	m_URLs["/lobby/login"] = JsonPacketHandler::login;
}

bool JsonPacketHandler::readJsonPacket(mln::Connection::sptr conn, unsigned int size, mln::CircularStream& msg)
{
	if (packetLobby::PT_JSON::HEADER_SIZE > size) {
		LOGE("invalid packet.");
		conn->closeReserve(0);
		return false;
	}

	packetLobby::PT_JSON req;
	msg.read((char*)&req, packetLobby::PT_JSON::HEADER_SIZE);

	if (packetLobby::PT_JSON::MAX_BODY_SIZE < req.bodySize
		|| 0 >= req.bodySize) {
		LOGE("body size error. sessoinId:{}, size:{}", conn->getIdentity(), req.bodySize);
		return false;
	}

	try {
		msg.read((char*)req.jsonBody, req.bodySize);
	}
	catch (std::exception& e) {
		LOGE("body pop error. sessoinId:{}, msg:{}", conn->getIdentity(), e.what());
		return false;
	}

	Json::Reader reader;
	Json::Value value;
	std::string serializedString((char*)&(req.jsonBody), req.bodySize);

	if (false == reader.parse(serializedString, value)) {
		LOGE("invalid json string");
		return false;
	}

#ifdef _DEBUG
	LOGD("json : {}", value.toStyledString());
#endif

	char urlString[sizeof(req.url)] = { 0, };
	memcpy(urlString, req.url, sizeof(req.url));

	value[RSP_SEQ] = req.sequenceNo;
	dispatch(conn, urlString, value);

	return true;
}

void JsonPacketHandler::dispatch(mln::Connection::sptr conn, const std::string& url, Json::Value& jv)
{
	auto exceptionHandler = [&conn, &url, &jv]() {
		GET_USER(conn, spUserBase, user);
		if (!user) {
			LOGW("invalid json parsing.");
		}
		else {
			LOGE("invalid json parsing. userID:{}, url:{}, requestBody:{}"
				, user->GetUserID(), url, jv.toStyledString());
		}
	};

	auto it = m_URLs.find(url);

	if (m_URLs.end() != it) {
		try {
			it->second(conn, url, jv);
			return;
		}
		catch (Json::Exception& e) {
			LOGE(e.what());
			exceptionHandler();
		}
		catch (...) {
			exceptionHandler();
		}
	}
	LOGE("invalid URL:{}", url);
}


void JsonPacketHandler::login(mln::Connection::sptr conn, const std::string& url, Json::Value& jv)
{
	GET_USER(conn, spUserBase, user);
}