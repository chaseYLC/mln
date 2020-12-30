#pragma once

#include <net/connection.h>
#include <net/json/json.h>
#include <net/logManager.h>
#include <net/messageProcedure.h>
#include <net/netService.h>
#include <net/netServiceConnector.h>
#include <net/user/userBasis.h>


#include <net/packets/packetJson.h>
#include <net/packets/packetParserJson.h>
#include <user/lobbyUser.h>

namespace mlnserver {

	class SampleConnector
	{
	public:
		void onConnect(mln::Connection::sptr conn) {
			LOGD("onConnect - {}/{}"
				, conn->socket().remote_endpoint().address().to_string()
				, conn->socket().remote_endpoint().port());

			// create user.
			std::shared_ptr<mln::UserBasis> user = std::make_shared<User>(conn);
			conn->setUser(user);

			auto userSample = std::static_pointer_cast<User>(user);

			// send test-packet.
			Json::Value req;
			req["myId"] = "MyUniqueId";
			userSample->sendJsonPacket("/lobby/login", req);
		}

		void onConnectFailed(mln::Connection::sptr spConn) {
			LOGW("onConnectFailed");
		}

		void onClose(mln::Connection::sptr conn) {
			LOGD("onClose - {}/{}"
				, conn->socket().remote_endpoint().address().to_string()
				, conn->socket().remote_endpoint().port());
		}
		void onCloseFailed(mln::Connection::sptr spConn) {}

		void onUpdate(uint64_t elapse) {}
		void onExpiredSession(mln::Connection::sptr spConn) {}
		void noHandler(mln::Connection::sptr spConn, mln::CircularStream& msg) {}

	public:
		void initHandler(mln::MessageProcedure* msgProc)
		{
			msgProc->registMessage(packetJson::PT_JSON::packet_value, &SampleConnector::readJsonPacket);

			// lobby
			m_URLs["/lobby/login"] = SampleConnector::login;
		}

		static bool readJsonPacket(mln::Connection::sptr conn, unsigned int size, mln::CircularStream& msg)
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

		static void dispatch(mln::Connection::sptr conn, const std::string& url, Json::Value& jv)
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


		static void login(mln::Connection::sptr conn, const std::string& url, Json::Value& jv)
		{
			GET_USER(conn, spUserBase, user);

			LOGD("received login-ack. msg:{}", jv[RSP_RM].asString());
		}

		static void tryConnect(boost::asio::io_context& ioc, const int32_t port)
		{
			static SampleConnector connectorInstance;

			mln::EventReceiverConnectorRegister<SampleConnector> 
				connectorHandler(&connectorInstance);

			mln::NetService::ServiceParams
				serviceInitParamsForClnt(ioc
					, connectorHandler
					, mln::PacketJsonParser::packetParser
					, mln::PacketJsonParser::getMsgManipulator()
					, 1000
					, 0
				);

			mln::NetService::ConnectorUserParams connectorUserParam(
				"127.0.0.1"
				, std::to_string(port)
			);

			auto svcConnector = mln::NetService::createConnector(
				serviceInitParamsForClnt
				, connectorUserParam
			);

			svcConnector->connectWait(
				1	// session count
				, 0	// 
			);
		}

		private:
			inline static std::map<std::string
				, std::function< void(mln::Connection::sptr, const std::string&, Json::Value&) > > m_URLs;
	};
	
}//namespace mlnserver {

