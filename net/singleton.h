#pragma once

#include <mutex>

namespace MLN
{
	namespace Net
	{
		template< typename T >
		class Singleton
		{
		public:
			__declspec(noinline) static T* instance()
			{
				std::call_once(_singleton_once_flag
					, [&] {
					_instance = new T;
				});

				return _instance;
			}
			Singleton() = default;
			__declspec(noinline) static void Release()
			{
				if (nullptr != _instance)
				{
					delete _instance;
					_instance = nullptr;
				}
			}

			virtual ~Singleton() = default;

		private:
			static T* _instance;
			Singleton(Singleton const&) = delete;
			Singleton& operator = (Singleton const&) = delete;

			static std::once_flag _singleton_once_flag;
		};

		template< typename T > T* Singleton<T>::_instance = nullptr;
		template< typename T > std::once_flag Singleton<T>::_singleton_once_flag;




		template< typename T >
		class SingletonLight
		{
		public:
			__declspec(noinline) static T* instance()
			{
				if (nullptr == _instance) {
					_instance = new T;
				}
				return _instance;
			}

			SingletonLight() = default;

			__declspec(noinline) static void release()
			{
				if (nullptr != _instance)
				{
					delete _instance;
					_instance = nullptr;
				}
			}

			virtual ~SingletonLight() = default;

		private:
			static T* _instance;
			SingletonLight(SingletonLight const&) = delete;
			SingletonLight& operator = (SingletonLight const&) = delete;
		};

		template< typename T > T* SingletonLight<T>::_instance = nullptr;

	};//
};
