#pragma once

#include <net/user/userBasis.h>
#include <net/simdjson.h>

namespace mln
{
	template <typename T>
	struct DBResult
	{
		bool isSuccess;
		T data;
	};

	class RestClient
	{
	public:
		bool RequestJson(std::string jsonStr, std::string& ret)
		{
			try
			{
				////////////////////////////////////////////
				// was ÀÀ´ä°ª
				ret = "{\"connectUID\":1,\"accountType\":2}";
				////////////////////////////////////////////

				return true;
			}
			catch (std::exception e)
			{
				return false;
			}
		}
	};

	class DBManager
	{
	public:
		template <typename Result>
		bool QuerySync(std::string query, Result& ret)
		{
			return restClient.RequestJson(query, ret);
		}

		template <typename User, typename Result>
		void ReserveQuery(std::string query, std::shared_ptr<User> user, std::function<void(std::shared_ptr<User>, std::shared_ptr<DBResult<Result>>)> completionCallback)
		{
			auto req = [this, query]()
			{
				std::shared_ptr<DBResult<Result>> ret = std::make_shared<DBResult<Result>>();
				ret->isSuccess = restClient.RequestJson(query, ret->data);
				return ret;
			};

			boost::shared_future<std::shared_ptr<DBResult<Result>>> reqFut = boost::async(req);

			boost::shared_future<void> callbackFut = reqFut.then(
				[user, completionCallback](boost::shared_future<std::shared_ptr<DBResult<Result>>> ret) {

					auto conn = user->_conn.lock();
					
					if (conn)
					{
						boost::asio::post(
							boost::asio::bind_executor(conn->strand(),
								boost::bind(completionCallback, user, ret.get())
							));
					}

					//(completionCallback)(user, ret.get());
				});
		}

	private:
		RestClient restClient;
	};
}