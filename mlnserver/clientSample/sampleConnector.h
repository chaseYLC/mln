#pragma once

#include <net/connection.h>
#include <net/json/json.h>
#include <net/logManager.h>
#include <net/messageProcedure.h>
#include <net/netService.h>
#include <net/netServiceConnector.h>
#include <net/user/userBasis.h>



#include <packetLobby.h>
#include <packetParserJson.h>

namespace mlnserver {

	class SampleUser
		: public mln::UserBasis
	{
	public:
		SampleUser(mln::Connection::sptr conn)
			: UserBasis(conn)
		{}

		std::string GetUserID() const {
			return "";
		}

		int sendJsonPacket(const std::string& url, Json::Value& jv)
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
	};

	class SampleConnector
	{
	public:
		void onConnect(mln::Connection::sptr conn) {
			LOGD("onConnect - {}/{}"
				, conn->socket().remote_endpoint().address().to_string()
				, conn->socket().remote_endpoint().port());

			// create user.
			std::shared_ptr<mln::UserBasis> user = std::make_shared<SampleUser>(conn);
			conn->setUser(user);

			auto userSample = std::static_pointer_cast<SampleUser>(user);

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
			/*using namespace RECORDER_PACKET_PROTOCOL;

			msgProc->registMessage(SETTING_ACK::packet_value, &SampleConnector::hSETTING_ACK);
			msgProc->registMessage(RECORD_DATA::packet_value, &SampleConnector::hRECORD_DATA);*/
		}

		static bool hTestACK(mln::Connection::sptr conn, unsigned int size, mln::CircularStream& msg)
		{
			return true;
		}

		static bool hTestDataAck(mln::Connection::sptr conn, unsigned int size, mln::CircularStream& msg)
		{
			/*using namespace RECORDER_PACKET_PROTOCOL;

			RECORD_DATA* data = (RECORD_DATA *)msg.data();
			std::string packetString;
			packetString.reserve(data->bodySize);
			packetString.resize(data->bodySize);

			std::memcpy( (void*)(packetString.c_str()), (void*)(&data->body[0]), (size_t)data->bodySize);

			LOGD("Test Received. msg:{}", packetString);*/

			return true;
		}

		static void tryConnect(boost::asio::io_context& ioc, const int32_t port)
		{
			static SampleConnector connectorInstance;

			mln::EventReceiverConnectorRegister<SampleConnector> 
				connectorHandler(&connectorInstance);

			mln::NetService::ServiceParams
				serviceInitParamsForClnt(ioc
					, connectorHandler
					, mlnserver::PacketJsonParser::packetParser
					, mlnserver::PacketJsonParser::getMsgManipulator()
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

	};
	
}//namespace mlnserver {

