#include "stdafx.h"
#include "lobbyUserManager.h"

#include <net/container/lockfreeLinkedlist.h>
#include <net/logManager.h>
#include "lobbyUser.h"

#pragma warning(disable: 4311)	// for lockfreeLinkedlist

using namespace mlnserver;

LobbyUserManager::LobbyUserManager()
{
	m_connectedUserList = std::make_unique<LockFreeLinkedList<KeySessionPair>>();
}

bool LobbyUserManager::addUser(mln::Connection::sptr conn)
{
	/*return UserManagerBasis::addUserBasis(
		conn
		, [&conn]() {return new mlnserver::User(conn); }
		, [&conn](mln::UserBasis* userBasis) {delete userBasis; }
	);*/

	return UserManagerBasis::addUserBasis<User>(conn);
}

void LobbyUserManager::closedUser(mln::Connection::sptr conn)
{
	/*return UserManagerBasis::deleteUserBasis(
		conn
		, [&conn](mln::UserBasis* userBasis) {delete userBasis; }
	);*/

	auto spUser = std::static_pointer_cast<User>(conn->getUser());
	//auto spRoom = spUser->m_wpRoom.lock();
	//if (spRoom) {
	//	boost::asio::dispatch(boost::asio::bind_executor(
	//		*spRoom->GetStrand().get()
	//		, boost::bind(&Room::Leave, spRoom.get()
	//			, spUser->userKey
	//			, true // noti to others
	//		)));
	//}
	//else {
	//	UserManagerBasis::deleteUserBasis<User>(conn);
	//}

	UserManagerBasis::deleteUserBasis<User>(conn);
	DeleteUserKey(spUser->m_userIDX);
}

void LobbyUserManager::deleteUser(mln::Connection::sptr conn)
{
	UserManagerBasis::deleteUserBasis<User>(conn);
}

std::tuple<bool, size_t> LobbyUserManager::InsertUserKey(const UserKeyType userKey, const int64_t sessionKey)
{
	if (m_connectedUserList->Insert({ userKey, sessionKey })) {
		return std::make_tuple(true, m_connectedUserList->size());
	}
	else {
		return std::make_tuple(false, m_connectedUserList->size());
	}
}

void LobbyUserManager::DeleteUserKey(const UserKeyType userKey)
{
	m_connectedUserList->Delete({ userKey, 0 });
}

bool LobbyUserManager::FindUserKey(const UserKeyType userKey)
{
	return false;
}

bool LobbyUserManager::FindUserSessionKey(const UserKeyType userKey, int64_t& sessionKey) const
{
	KeySessionPair pairData{userKey, sessionKey};
	if (true == m_connectedUserList->Find(pairData)) {
		sessionKey = pairData.sessionKey_;
		return true;
	}
	return false;
}