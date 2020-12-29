#pragma once

#include <mutex>

namespace mln
{
	template< typename T >
	class Singleton
	{
	public:
#if defined(__GNUC__)
		__attribute__((noinline)) static T* instance()
#else
		__declspec(noinline) static T* instance()
#endif
		{
			std::call_once(_singleton_once_flag
				, [&] {
					_instance = new T;
				});

			return _instance;
		}
		Singleton() = default;
#if defined(__GNUC__)
		__attribute__((noinline)) static void Release()
#else
		__declspec(noinline) static void Release()
#endif
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
#if defined(__GNUC__)
		__attribute__((noinline)) static T* instance()
#else
		__declspec(noinline) static T* instance()
#endif
		{
			if (nullptr == _instance) {
				_instance = new T;
			}
			return _instance;
		}

		SingletonLight() = default;

#if defined(__GNUC__)
		__attribute__((noinline)) static void Release()
#else
		__declspec(noinline) static void Release()
#endif
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

};//namespace mln
