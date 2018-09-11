#pragma once

#include <boost/pool/pool.hpp>
#include <boost/thread/mutex.hpp>

namespace MLN
{
	namespace Net
	{
		template< typename __classname >
		class MemoryPool
		{
		public:
			static void* operator new(size_t size){
				boost::lock_guard<boost::mutex> lock(_mutex);
				__classname* ptr = static_cast<__classname*>(_thisPool.malloc());
				return ptr;
			}

			static void operator delete(void* ptr, size_t size) {
				boost::lock_guard<boost::mutex> lock(_mutex);
				_thisPool.free(ptr);
			}
			static void destruct(__classname* ptr) {
				delete ptr;
			}

		private:
			static boost::pool<> _thisPool;
			static boost::mutex _mutex;
		};

		template<typename __classname>
		boost::pool<> MemoryPool<__classname>::_thisPool(sizeof(__classname));

		template<typename __classname>
		boost::mutex MemoryPool<__classname>::_mutex;

	};//Net
};
