#pragma once

#include <utility>

namespace mln
{
	namespace Task
	{
		template <typename USER, typename RESULT>
		void ExecuteAsync(
			  std::shared_ptr<USER> user
			, std::function<void(std::shared_ptr<RESULT>)> task
			, std::function<void(std::shared_ptr<USER>, std::shared_ptr<RESULT>)> completionCallback)
		{
			auto req = [task]()
			{
				std::shared_ptr<RESULT> ret = std::make_shared<RESULT>();
				task(ret);
				return ret;
			};

			boost::shared_future<std::shared_ptr<RESULT>> reqFut = boost::async( req );

			boost::shared_future<void> callbackFut = reqFut.then(
				[user, completionCallback](boost::shared_future<std::shared_ptr<RESULT>> ret) {
			
					if (auto conn = user->_conn.lock(); conn){
						boost::asio::post(
							boost::asio::bind_executor(conn->strand(),
								boost::bind(completionCallback, user, ret.get())
							));
					}
				});
		}
	};
}