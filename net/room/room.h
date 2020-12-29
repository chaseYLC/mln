#pragma once

#include <memory>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>

#include "roomResult.h"
#include "../json/json.h"


namespace mln
{
	class Room
	{
	public:
		enum class RoomResult {
			SUCCESSED = 0,
			FAILED_ROOMKEY = -1,
			FAILED_JOIN_INROOM_ALREADY = -2,
			ROOM_RESULT_USER_VALUE_BEGIN = 1000,
			/////////////////////////////////////
		};

		Room(boost::asio::io_context& ioc, const uint64_t roomKey)
			/*: _updateTimer(ioc)*/
		{
			_strand = std::make_shared<boost::asio::io_context::strand>(ioc);
			m_roomKey = roomKey;
		}

		template <typename CREATE_PRTY>
		RoomResult SetRoomTemplate(const uint64_t roomKey, const CREATE_PRTY& createPrty)
		{
			return RoomResult::SUCCESSED;
		}

		uint64_t GetRoomKey() const { return m_roomKey; }

		template <typename JOIN_PRTY>
		RoomResult EnterTemplate(JOIN_PRTY &joinPrty)
		{
			joinPrty.user.strandRoom = _strand;
			return RoomResult::SUCCESSED;
		}

		void broadcastWithSelf(Json::Value& jv, const std::string& url)
		{
			/*for (size_t i = 0; i < m_gameUsersCount; ++i) {
				if (m_roomUsers[i].userPt) {
					m_roomUsers[i].userPt->SendJsonPacket(url, jv);
				}
			}*/
		}

		uint32_t getGameUsersCount() const { return m_gameUsersCount; }
		void setGameUsersCount(const uint32_t count){ m_gameUsersCount = count; }

		std::shared_ptr<boost::asio::io_context::strand> GetStrand() { return _strand; }

	protected:
		std::shared_ptr<boost::asio::io_context::strand> _strand;

		uint32_t m_gameUsersCount;
		uint64_t m_roomKey;
	};

}

