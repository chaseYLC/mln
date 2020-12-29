#pragma once

#include <atomic>
#include <functional>
#include <shared_mutex>
#include <unordered_map>

#include <boost/asio/io_context.hpp>

#include "room.h"
//#include "roomResult.h"
#include "../user/userBasis.h"
#include "../json/json.h"

namespace mln
{
	inline static std::atomic< uint64_t > s_roomIdentitySeed = { 1 };

	template<typename ROOM>
	class RoomManager
	{
	public:
		using RoomKey = uint64_t;
		using RoomContainer = std::unordered_map< uint64_t, std::shared_ptr<ROOM> >;
		typedef typename ROOM::RoomResult RoomResult;
		using ResultTuple = std::tuple<RoomResult, std::shared_ptr<ROOM>>;

	public:
		void SetIOC(boost::asio::io_context * ioc) { _ioc = ioc;}

		template <typename CREATE_PRTY>
		ResultTuple CreateRoom(const CREATE_PRTY& createPrty
			, std::function<RoomResult(const CREATE_PRTY& )> preHandler
			, std::function<void(const CREATE_PRTY& , ROOM*)> postHandler)
		{
			std::lock_guard wlock{ _lock };

			if (preHandler) {
				const RoomResult result = preHandler(createPrty);
				if (RoomResult::SUCCESSED != result) {
					return std::make_tuple(result, std::shared_ptr<ROOM>());
				}
			}

			const uint64_t roomKey = s_roomIdentitySeed.fetch_add(1, std::memory_order_relaxed);

			auto room = std::make_shared<ROOM>(*_ioc, roomKey);
			room->SetRoom(createPrty);
			if (false == _rooms.emplace(roomKey, room).second) {
				return std::make_tuple(RoomResult::FAILED_ROOMKEY, std::shared_ptr<ROOM>());
			}

			if (postHandler) {
				const RoomResult result = postHandler(createPrty, room.get() );
				if (RoomResult::SUCCESSED != result) {
					return std::make_tuple(result, room);
				}
			}

			return std::make_tuple(RoomResult::SUCCESSED, room);
		}

		template <typename ROOM_MGR, typename CREATE_PRTY>
		ResultTuple CreateRoom(ROOM_MGR *roomMgr, const CREATE_PRTY& createPrty)
		{
			std::lock_guard wlock{ _lock };

			/*auto func = std::bind(&ROOM_MGR::PreCreateRoom, this, std::placeholders::_1);
			func(createPrty);*/
			RoomResult result = roomMgr->PreCreateRoom(createPrty);

			/*const RoomResult result = std::bind(&ROOM_MGR::PreCreateRoom, this, std::placeholders::_1)(createPrty);*/
			if (RoomResult::SUCCESSED != result) {
				return std::make_tuple(result, std::shared_ptr<ROOM>());
			}

			const uint64_t roomKey = GenerateRoomKey();

			auto room = std::make_shared<ROOM>(*_ioc, roomKey);
			room->SetRoom(createPrty);
			if (false == _rooms.emplace(roomKey, room).second) {
				return std::make_tuple(RoomResult::FAILED_ROOMKEY, std::shared_ptr<ROOM>());
			}

			/*std::bind(&ROOM_MGR::PostCreateRoom, this, std::placeholders::_1, std::placeholders::_2)(createPrty, room.get());*/
			result = roomMgr->PostCreateRoom(createPrty, room.get());
			if (RoomResult::SUCCESSED != result) {
				return std::make_tuple(result, room);
			}

			return std::make_tuple(RoomResult::SUCCESSED, room);
		}


		template <typename JOIN_PRTY>
		ResultTuple JoinRoom(const RoomKey roomKey, JOIN_PRTY& joinPrty
			, std::function<RoomResult(JOIN_PRTY&, ROOM*)> preHandler = nullptr
			, std::function<void(JOIN_PRTY&, ROOM*)> postHandler = nullptr)
		{
			std::shared_lock rlock{ _lock };

			if (auto it = _rooms.find(roomKey); _rooms.end() != it) {
				RoomResult result;

				if (preHandler) {
					const RoomResult result = preHandler(joinPrty, it->second.get());
					if (RoomResult::SUCCESSED != result) {
						return std::make_tuple(result, it->second);
					}
				}

				result = it->second->Join(joinPrty);
				if (RoomResult::SUCCESSED != result) {
					return std::make_tuple(result, it->second);
				}

				if (postHandler) {
					postHandler(joinPrty, it->second.get());
				}

				return std::make_tuple(RoomResult::SUCCESSED, it->second);
			}
			else {
				return std::make_tuple(RoomResult::FAILED_ROOMKEY, std::shared_ptr<ROOM>());
			}
		}

		template <typename ROOM_MGR, typename JOIN_PRTY>
		ResultTuple JoinRoom(ROOM_MGR* roomMgr, const RoomKey roomKey, JOIN_PRTY& joinPrty)
		{
			std::shared_lock rlock{ _lock };

			if (auto it = _rooms.find(roomKey); _rooms.end() != it) {
				RoomResult result = roomMgr->PreJoinRoom(joinPrty, it->second.get());

				if (RoomResult::SUCCESSED != result) {
					return std::make_tuple(result, it->second);
				}

				result = it->second->Join(joinPrty);
				if (RoomResult::SUCCESSED != result) {
					return std::make_tuple(result, it->second);
				}

				roomMgr->PostJoinRoom(joinPrty, it->second.get());
				return std::make_tuple(RoomResult::SUCCESSED, it->second);
			}
			else {
				return std::make_tuple(RoomResult::FAILED_ROOMKEY, std::shared_ptr<ROOM>());
			}
		}

		template <typename ROOM_MGR, typename JOIN_PRTY>
		ResultTuple JoinRoom(ROOM_MGR* roomMgr, std::shared_ptr<ROOM> spRoom, JOIN_PRTY& joinPrty)
		{
			RoomResult result = roomMgr->PreJoinRoom(joinPrty, spRoom.get());

			if (RoomResult::SUCCESSED != result) {
				return std::make_tuple(result, spRoom);
			}

			result = spRoom->Join(joinPrty);
			if (RoomResult::SUCCESSED != result) {
				return std::make_tuple(result, spRoom);
			}

			roomMgr->PostJoinRoom(joinPrty, spRoom.get());
			return std::make_tuple(RoomResult::SUCCESSED, spRoom);
		}

		std::optional<std::shared_ptr<ROOM>> GetRoom(const size_t roomKey)
		{
			std::shared_lock rlock{ _lock };

			auto it = _rooms.find(roomKey);
			if (_rooms.end() == it) {
				return std::nullopt;
			}
			return it->second;
		}


		//static void Broadcast(std::shared_ptr<boost::asio::io_context::strand> spRoomStrand
		//	, UserBasis* user, const std::string& url, Json::Value& jv)
		//{
		//	/*jv[RSP_RC] = 0;
		//	jv[RSP_RM] = RSP_OK;*/

		//	boost::asio::post(boost::asio::bind_executor(*spRoomStrand.get()
		//		, boost::bind(&Room::broadcastWithSelf
		//			, user->_room
		//			, jv
		//			, url
		//		)));
		//}

		size_t GetRoomCount() const {
			std::shared_lock<std::shared_mutex> rlock(_lock);
			return _rooms.size();
		}

		/*template<typename ROOM_TYPE>
		void FunctionForeachImmutable(std::function< void(std::shared_ptr<ROOM_TYPE>)> callback) const
		{
			std::shared_lock<std::shared_mutex> rlock(_lock);

			for (auto& [k, spRoom] : _rooms) {
				if (spRoom) {
					callback(spRoom);
				}
			}
		}*/
		void FunctionForeachImmutable(std::function< void(std::shared_ptr<ROOM>)> callback) const
		{
			std::shared_lock<std::shared_mutex> rlock(_lock);

			for (auto& [k, spRoom] : _rooms) {
				if (spRoom) {
					callback(spRoom);
				}
			}
		}

		int FunctionTargetImmutable(std::function< void(std::shared_ptr<ROOM>)> callback
			, const int64_t roomKey) const
		{
			std::shared_lock<std::shared_mutex> rlock(_lock);

			if(auto it = _rooms.find(roomKey); _rooms.end() != it) {
				if (it->second) {
					callback(it->second);
					return 0;
				}
			}
			return -1;
		}

	protected:
		uint64_t GenerateRoomKey() const {
			return s_roomIdentitySeed.fetch_add(1, std::memory_order_relaxed);
		}



	protected:
		mutable std::shared_mutex _lock;
		boost::asio::io_context * _ioc = nullptr;

		RoomContainer _rooms;
	};


}//namespace mln

