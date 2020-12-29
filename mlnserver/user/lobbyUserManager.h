#pragma once

#include <functional>
#include <map>
#include <memory>
#include <tuple>

#include <net/singleton.h>
#include <net/user/userManagerBasis.h>

namespace mlnserver {
	class User;
}

template<typename T>
class LockFreeLinkedList;

class LobbyUserManager
	: public mln::UserManagerBasis
	, public mln::SingletonLight<LobbyUserManager>
{
public:
	using UserKeyType = int64_t;

	struct KeySessionPair
	{
		UserKeyType userKey_;
		int64_t sessionKey_;

		bool operator < (const KeySessionPair rhs) const{
			return userKey_ < rhs.userKey_;
		}
	};

public:
	LobbyUserManager();

	bool addUser(mln::Connection::sptr conn);
	void closedUser(mln::Connection::sptr conn);
	void deleteUser(mln::Connection::sptr conn);
	

	std::tuple<bool, size_t> InsertUserKey(const UserKeyType userKey, const int64_t sessionKey);
	void DeleteUserKey(const UserKeyType userKey);
	bool FindUserKey(const UserKeyType userKey);
	bool FindUserSessionKey(const UserKeyType userKey, int64_t &sessionKey) const;

private:
	std::unique_ptr<LockFreeLinkedList<KeySessionPair>> m_connectedUserList;
};
