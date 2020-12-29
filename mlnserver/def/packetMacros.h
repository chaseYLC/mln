#pragma once 

#define MAKE_PACKET_RESULT_OK			std::make_tuple(true, "")
#define MAKE_PACKET_RESULT_FAILED(msg)	std::make_tuple(false, msg)

#define PACKET_STRING_SIZE_CHECK(stringValueName, size)\
	if (stringValueName.length() > size) {\
		return std::make_tuple(false, fmt::format("invalid param. name:{}, length:{} max:{}", #stringValueName, stringValueName.length(), size));\
}

#define PACKET_STRING_SIZE_CHECK_MINMAX(stringValueName, minSize, maxSize)\
	if (stringValueName.length() > maxSize || stringValueName.length() <= minSize ) {\
		return std::make_tuple(false, fmt::format("invalid param. name:{}, length:{} min:{}, max:{}", #stringValueName, stringValueName.length(), minSize, maxSize));\
}

#define PACKET_INT_SIZE_CHECK_MINMAX(intValueName, minSize, maxSize)\
	if (intValueName > maxSize || intValueName <= minSize ) {\
		return std::make_tuple(false, fmt::format("invalid param. name:{}, value:{}, min:{}, max:{}", #intValueName, intValueName, minSize, maxSize));\
}




#define GET_USER(user)	std::shared_ptr<User> user = std::static_pointer_cast<User>(userBasis)

#define CHECK_AUTH	if (false == user->m_userAuthState.IsAuthenticated()) {\
						LOGE("<packetHandler|room> not certified user.");\
						user->CloseReserve(0);\
						return false;\
					}


#define CONCATENATE_ROOM_FUNCNAME(F) Room:: F

#define ROOM_SYNC_USER_EXEC1(user, FuncName, p1)	\
if (auto spRoom = user->m_wpRoom.lock(); spRoom) {\
	boost::asio::dispatch(boost::asio::bind_executor(\
		*spRoom->GetStrand().get(),\
			boost::bind(&CONCATENATE_ROOM_FUNCNAME(FuncName), spRoom.get()\
			, p1\
	)));\
}

#define ROOM_SYNC_USER_EXEC2(user, FuncName, p1, p2)	\
if (auto spRoom = user->m_wpRoom.lock(); spRoom) {\
	boost::asio::dispatch(boost::asio::bind_executor(\
		*spRoom->GetStrand().get(),\
			boost::bind(&CONCATENATE_ROOM_FUNCNAME(FuncName), spRoom.get()\
			, p1, p2\
	)));\
}

#define ROOM_SYNC_USER_EXEC3(user, FuncName, p1, p2, p3)	\
if (auto spRoom = user->m_wpRoom.lock(); spRoom) {\
	boost::asio::dispatch(boost::asio::bind_executor(\
		*spRoom->GetStrand().get(),\
			boost::bind(&CONCATENATE_ROOM_FUNCNAME(FuncName), spRoom.get()\
			, p1, p2, p3\
	)));\
}

#define ROOM_SYNC_USER_EXEC4(user, FuncName, p1, p2, p3, p4)	\
if (auto spRoom = user->m_wpRoom.lock(); spRoom) {\
	boost::asio::dispatch(boost::asio::bind_executor(\
		*spRoom->GetStrand().get(),\
			boost::bind(&CONCATENATE_ROOM_FUNCNAME(FuncName), spRoom.get()\
			, p1, p2, p3, p4\
	)));\
}

#define ROOM_SYNC_USER_EXEC5(user, FuncName, p1, p2, p3, p4, p5)	\
if (auto spRoom = user->m_wpRoom.lock(); spRoom) {\
	boost::asio::dispatch(boost::asio::bind_executor(\
		*spRoom->GetStrand().get(),\
			boost::bind(&CONCATENATE_ROOM_FUNCNAME(FuncName), spRoom.get()\
			, p1, p2, p3, p4, p5\
	)));\
}

#define ROOM_SYNC_ROOM_EXEC2(spRoom, FuncName, p1, p2)	\
	boost::asio::dispatch(boost::asio::bind_executor(\
		*spRoom->GetStrand().get(),\
			boost::bind(&CONCATENATE_ROOM_FUNCNAME(FuncName), spRoom.get()\
			, p1, p2\
	)));

