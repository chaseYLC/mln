#pragma once

#include <memory>
#include <string>

#include <net/connectionImpl.h>
#include <net/logManager.h>
#include <net/memoryPool.h>
#include <net/time/scheduler.h>
#include <net/time/simpleScheduler.h>
#include <net/task/task.h>

#include <user/lobbyUser.h>
#include <user/lobbyUserManager.h>


class TestFrameworks 
{
public:
	static void TestUser()
	{
		// create connection
		auto conn = mln::ConnectionImpl::createDummyTestConnection(*shared_ios.get());
		auto currentId = conn->getIdentity();

		// add user
		LobbyUserManager::instance()->addUser(conn);
		GET_USER_LOBBY(user);
		LOGD("userID:{}", user->GetUserID());

		// test for targetUser
		LobbyUserManager::instance()->functionTarget(
			[&](mln::Connection::sptr conn) {
				LOGD("called. sessionID:{}", conn->getConnectionID());
				GET_USER_LOBBY(user);
				//user->m_userID = "targetUser";
			}
			, currentId
		);

		// test for allUser
		LobbyUserManager::instance()->functionForeach(
			[&](mln::Connection::sptr conn) {
				LOGD("called. sessionID:{}", conn->getConnectionID());
				GET_USER_LOBBY(user);
				//user->m_userID = "allUser";
		});

		/*LOGD("userID:{}", user->m_userID);*/

		// get User
		if (LobbyUserManager::instance()->getUser(currentId).has_value()) {
			auto spUserConn = LobbyUserManager::instance()->getUser(currentId).value().lock();

			if (spUserConn) {
				LOGD("spUserConn id:{}", spUserConn->getIdentity());

				GET_USER_LOBBY_BY_CONN(newUser, spUserConn);
				//newUser->m_userID = "allUser";
			}
		}

		// delete User
		LobbyUserManager::instance()->deleteUser(conn);
	}

	static void TestScheduler()
	{
		static mln::Scheduler<5> scheduler(*shared_ios.get());
		scheduler.AddJobInterval(1000
			, [](uint64_t d) {
				LOGD("Scheduler job1. dur:1000");
			}
		);

		scheduler.AddJobInterval(3000
			, [](uint64_t d) {
				LOGD("Scheduler job2. dur:3000");
			}
		);
	}

	static void TestSimpleScheduler()
	{
		{
			static mln::SimpleScheduler<5, 1000, false> scheduler(*shared_ios.get());

			scheduler.Init();
			scheduler.AddJobInterval(1
				, [](uint64_t d) {
					LOGD("SimpleScheduler job1. dur: 1000 * 1");
				}
			);
			scheduler.TimerStart();
		}

		{
			static mln::SimpleScheduler<5, 1000, true> scheduler2(*shared_ios.get());
			static auto spSt = std::make_shared<boost::asio::io_context::strand>(*shared_ios.get());

			scheduler2.Init(spSt);
			scheduler2.AddJobInterval(3
				, [](uint64_t d) {
					LOGD("SimpleScheduler job2. dur:1000 * 3");
				}
			);
			scheduler2.TimerStart();
		}
	}

	static void TestAsyncTask()
	{
		// create connection
		auto conn = mln::ConnectionImpl::createDummyTestConnection(*shared_ios.get());
		auto currentId = conn->getIdentity();

		// add user
		LobbyUserManager::instance()->addUser(conn);
		auto spUser = std::static_pointer_cast<mlnserver::User>(conn->getUser());
		LOGD("userID:{}", spUser->GetUserID() );



		struct MyResultType {
			int retCode;
			std::string retData;
		};

		struct MyDBClient {
			void RequesA(std::shared_ptr<MyResultType> outResult)
			{
				// http request~~
				//
				// set response.
				outResult->retCode = 0;
				outResult->retData = "abcd";
			}
		};
		static MyDBClient myDbClient;


		// 람다로 발송하거나,
		mln::Task::ExecuteAsync<mlnserver::User, MyResultType>(spUser
			, [](std::shared_ptr<MyResultType> outResult) {
				LOGD("run task~~");
				// 여기서 db requet 를 보내고 응답을 outResult 에 저장합니다.
				// DBManager->RequestAuth
				outResult->retCode = 0;
				outResult->retData = "abcd";
			}
			, [](std::shared_ptr<mlnserver::User> user, std::shared_ptr<MyResultType> result ) {
				// completion callback
				LOGD("task completed");
				LOGD("retCode:{}, retData:{}", result->retCode, result->retData);
			}
			);

		// DBClient 만들어서 발송하거나
		mln::Task::ExecuteAsync<mlnserver::User, MyResultType>(spUser
			, std::bind(&MyDBClient::RequesA, &myDbClient, std::placeholders::_1)
			, [](std::shared_ptr<mlnserver::User> user, std::shared_ptr<MyResultType> result) {
				// completion callback
				LOGD("task completed");
				LOGD("retCode:{}, retData:{}", result->retCode, result->retData);
			}
			);

	}


	static void Play() {
		//TestUser();
		//TestRoom();
		//TestScheduler();
		//TestSimpleScheduler();
		//TestRoomTick();
		TestAsyncTask();
	}
};