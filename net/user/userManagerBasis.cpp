#include "stdafx.h"

#include "../logManager.h"
#include "../messageBuffer.h"

#include "userManagerBasis.h"
#include "userBasis.h"

namespace mln {
	bool UserManagerBasis::createUser(const mln::EncType::Type encType, void* headerPtr, mln::Connection::sptr conn, uint32_t size, mln::MessageBuffer& msg)
	{
		//if (conn->getTag() != nullptr)
		//{
		//	LOGE << "created userobj already. ident: " << conn->getIdentity();
		//	return false;
		//}

		//chase_lobbysvr::User *user = new chase_lobbysvr::User(conn);
		//conn->setTag(user);


		//switch (encType)
		//{
		//case mln::EncType::GREETING:
		//case mln::EncType::AES_128_CBC:
		//case mln::EncType::AES_128_CBC_keR:
		//{
		//	// send key
		//	if (false == user->initEncKey(encType, nullptr, msg.data(), size)) {
		//		return false;
		//	}
		//}break;

		//default:
		//	return false;
		//}//switch (encType)

		// // sync key.
		//conn->setSyncEncryptionKey(encType, true);



		//// ���� �ð� ����
		////ROOM_PROTOCOL::PT_SERVER_TIME serverTime;
		////typedef std::decay<decltype(ROOM_PROTOCOL::PT_SERVER_TIME::timeOnServer)>::type timeType;
		////serverTime.timeOnServer = (timeType)time(NULL);
		////user->Send(serverTime);

		return true;
	}

	bool UserManagerBasis::decryptMessage(const mln::EncType::Type encType, mln::Connection::sptr conn, uint32_t msgSize, mln::MessageBuffer& msg, mln::MessageBuffer& madeMsg)
	{
		//chase_lobbysvr::User *user = (chase_lobbysvr::User *)conn->getTag();
		//if (nullptr == user)
		//{
		//	LOGE << "user is null. ident: " << conn->getIdentity();
		//	return false;
		//}

		//madeMsg.clear();

		//const int decryptedSize = user->Decrypt(encType, msg.data(), msgSize, madeMsg.enableBuffer(), (uint32_t) madeMsg.remainWriteSize());
		//if (0 >= decryptedSize) {
		//	LOGE << "failed Decryption. ident: " << conn->getIdentity();
		//	madeMsg.clear();
		//	return false;
		//}
		//madeMsg.write(decryptedSize);

		return true;
	}

	void UserManagerBasis::onClose(mln::Connection::sptr conn)
	{
		//chase_lobbysvr::User *user = (chase_lobbysvr::User *)conn->getTag();
		//if (nullptr == user) {
		//	return;
		//}

		//delete user;
		//conn->setTag(nullptr);
	}

	void UserManagerBasis::ErrorLog(const std::string& msg)
	{
		LOGE(msg);
	}

	bool UserManagerBasis::addUserBasis(mln::Connection::sptr conn
		, AllcoUserType allocUser
		, FreeUserType freeUser
	)
	{
		if (conn->getTag() != nullptr){
			LOGE("created userobj already. ident:{}", conn->getIdentity());
			return false;
		}

		UserBasis* user = allocUser();
		conn->setTag(user);

		std::unique_lock<std::shared_mutex> wlock(m_mtx_users);
		if (false == m_users.emplace(std::make_pair(conn->getIdentity(), conn)).second) {
			LOGE("failed insert user. identity : {}", conn->getIdentity());
			if (freeUser) {
				freeUser(user);
			}
			return false;
		}

		return true;
	}

	void UserManagerBasis::deleteUserBasis(mln::Connection::sptr conn
		, FreeUserType freeUser
	)
	{
		UserBasis* user = (UserBasis*)conn->getTag();
		if (nullptr == user) {
			return;
		}

		std::unique_lock<std::shared_mutex> wlock(m_mtx_users);

		auto it = m_users.find(conn->getIdentity());
		if (m_users.end() == it) {
			LOGE("none user. userIdentity : {}", conn->getIdentity());
			if (freeUser) {
				freeUser(user);
			}
			conn->setTag(nullptr);
			return;
		}

		m_users.erase(it);

		freeUser(user);

		conn->setTag(nullptr);
	}

	std::optional<mln::Connection::wptr> UserManagerBasis::getUser(const size_t identity)
	{
		std::shared_lock<std::shared_mutex> rlock(m_mtx_users);

		auto pair = m_users.find(identity);
		if (m_users.end() == pair) {
			return std::nullopt;
		}

		return pair->second;
	}

	int UserManagerBasis::functionTarget(FuncTargetType callback, const size_t sessionKey)
	{
		std::shared_lock<std::shared_mutex> rlock(m_mtx_users);

		auto it = m_users.find(sessionKey);
		if (m_users.end() == it) {
			return -1;
		}

		if (it->second) {
			callback(it->second);
		}
		return 0;
	}

	int UserManagerBasis::functionTarget(FuncTargetType callback, const std::vector<size_t>& sessionKeys)
	{
		std::shared_lock<std::shared_mutex> rlock(m_mtx_users);

		for (auto sessionId : sessionKeys) {
			if (m_users.end() == m_users.find(sessionId)) {
				return -1;
			}
		}

		for (auto sessionKey : sessionKeys) {
			auto it = m_users.find(sessionKey);
			assert(m_users.end() != it);

			if (it->second) {
				callback(it->second);
			}
		}
		return 0;
	}

	void UserManagerBasis::functionForeach(FuncTargetType callback)
	{
		std::shared_lock<std::shared_mutex> rlock(m_mtx_users);

		for (auto& [k, spUser] : m_users) {
			if (spUser) {
				callback(spUser);
			}
		}
	}

	size_t UserManagerBasis::getUserCount() const
	{
		std::shared_lock<std::shared_mutex> rlock(m_mtx_users);
		return m_users.size();
	}


}//namespace mln





