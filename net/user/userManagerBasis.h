#pragma once

#include <functional>
#include <map>
#include <shared_mutex>
#include <utility>

#include <fmt/core.h>

#include "../packetEncType.h"
#include "../connection.h"


namespace mln {

	class UserBasis;

	class UserManagerBasis
	{
	public:
		using AllcoUserType = std::function<UserBasis* ()>;
		using FreeUserType = std::function<void (UserBasis*)>;
		using FuncTargetType = std::function< void(mln::Connection::sptr)>;

		bool createUser(const mln::EncType::Type encType, void* headerPtr, mln::Connection::sptr conn, uint32_t size, mln::CircularStream& msg);
		bool decryptMessage(const mln::EncType::Type encType, mln::Connection::sptr conn, uint32_t msgSize, mln::CircularStream& msg, mln::CircularStream& madeMsg);
		
		bool addUserBasis(mln::Connection::sptr conn
			, AllcoUserType allocUser
			, FreeUserType freeUser
		);

		template < typename T >
		bool addUserBasis(mln::Connection::sptr conn)
		{
			if (conn->getUser()) {
				ErrorLog(fmt::format("created userobj already. ident:{}", conn->getIdentity()));
			}

			std::shared_ptr<UserBasis> user = allocUserBasis<T>(conn);
			conn->setUser(user);

			std::unique_lock<std::shared_mutex> wlock(m_mtx_users);
			if (false == m_users.emplace(std::make_pair(conn->getIdentity(), conn)).second) {
				ErrorLog(fmt::format("failed insert user. identity : {}", conn->getIdentity()));
				return false;
			}
			return true;
		}

		template < typename T >
		std::shared_ptr<UserBasis> allocUserBasis(mln::Connection::sptr conn)
		{
			return std::make_shared<T>(conn);
		}

		void deleteUserBasis(mln::Connection::sptr conn
			, FreeUserType freeUser
		);

		template < typename T >
		void deleteUserBasis(mln::Connection::sptr conn)
		{
			auto spUser = conn->getUser();
			if (!spUser) {
				return;
			}

			std::unique_lock<std::shared_mutex> wlock(m_mtx_users);

			auto it = m_users.find(conn->getIdentity());
			if (m_users.end() == it) {
				LOGE("none user. userIdentity : {}", conn->getIdentity());
				ErrorLog(fmt::format("none user. userIdentity : {}", conn->getIdentity()));
				conn->setUser(nullptr);
				return;
			}

			m_users.erase(it);

			conn->setUser(nullptr);
		}

		std::optional<mln::Connection::wptr> getUser(const size_t identity);

		int functionTarget(FuncTargetType callback, const size_t sessionKey);
		int functionTarget(FuncTargetType callback, const std::vector<size_t>& sessionKeys);
		void functionForeach(FuncTargetType callback);

		static void onClose(mln::Connection::sptr conn);

		size_t getUserCount() const;

	protected:
		void ErrorLog(const std::string& msg);
		

	protected:
		mutable std::shared_mutex m_mtx_users;

		std::map< size_t	// connection identity
			, mln::Connection::sptr > m_users;
	};
}//namespace mln {