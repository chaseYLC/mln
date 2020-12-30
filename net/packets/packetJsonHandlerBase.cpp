#include "stdafx.h"
#include "packetJsonHandlerBase.h"

#include <net/messageProcedure.h>
#include "packetJson.h"

using namespace mln;

void JsonPacketHandlerBase::initHandlers(mln::MessageProcedure* msgProc)
{
	msgProc->registMessage(packetJson::PT_JSON::packet_value, &JsonPacketHandlerBase::readJsonPacket);
}

bool JsonPacketHandlerBase::readJsonPacket(mln::Connection::sptr conn, unsigned int size, mln::CircularStream& msg)
{
	if (packetJson::PT_JSON::HEADER_SIZE > size) {
		LOGE("invalid packet.");
		conn->closeReserve(0);
		return false;
	}

	packetJson::PT_JSON req;
	msg.read((char*)&req, packetJson::PT_JSON::HEADER_SIZE);

	if (packetJson::PT_JSON::MAX_BODY_SIZE < req.bodySize
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

void mln::JsonPacketHandlerBase::dispatch(mln::Connection::sptr conn, const std::string& url, Json::Value& jv)
{
	auto exceptionHandler = [&conn, &url, &jv]() {
		LOGW("invalid json parsing. sessionId:{}", conn->getIdentity() );
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