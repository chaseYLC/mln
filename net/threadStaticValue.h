#pragma once

#include <type_traits>
#include <boost/lockfree/policies.hpp>

#ifndef I_DONT_NEED_RELEASE_ALLOCED_MEMORY_IN_THREADSTATICVALUE
#include <boost/lockfree/stack.hpp>
static const int DefaultThreadStaticValueCapacityLimit = 200;
#endif

#ifndef I_DONT_NEED_RELEASE_ALLOCED_MEMORY_IN_THREADSTATICVALUE
template < typename T
	, int UniqueID
	, unsigned int CapacityLimit = DefaultThreadStaticValueCapacityLimit >
#else
template < typename T, int UniqueID >
#endif
class ThreadStaticValue
{
public:

#ifndef I_DONT_NEED_RELEASE_ALLOCED_MEMORY_IN_THREADSTATICVALUE
	~ThreadStaticValue()
	{
		while (false == _ContainerForClear.empty()){
			T *ptr = nullptr;
			_ContainerForClear.pop(ptr);
			delete ptr;
		}
	}
#endif

	static T& get()
	{
		__declspec(thread) static T * threadValue = nullptr;
		if (nullptr == threadValue){
			threadValue = new T();
#ifndef I_DONT_NEED_RELEASE_ALLOCED_MEMORY_IN_THREADSTATICVALUE
			_ContainerForClear.push(threadValue);
#endif
		}
		return *threadValue;
	}

#ifndef I_DONT_NEED_RELEASE_ALLOCED_MEMORY_IN_THREADSTATICVALUE
	static boost::lockfree::stack<T *, boost::lockfree::capacity<CapacityLimit> >  _ContainerForClear;
#endif
};

#ifndef I_DONT_NEED_RELEASE_ALLOCED_MEMORY_IN_THREADSTATICVALUE
template < typename T
	, int UniqueID
	, unsigned int CapacityLimit = DefaultThreadStaticValueCapacityLimit >
	boost::lockfree::stack<T *, boost::lockfree::capacity<CapacityLimit> > ThreadStaticValue< T, UniqueID, CapacityLimit>::_ContainerForClear;
#endif




/*
	예제 코드.


	#include "stdafx.h"

#include <Base/includeLib.h>
#include <Base/Base/include/threadStaticValue.h>
#include <thread>

int _tmain(int argc, _TCHAR* argv[])
{
	// 객체를 쓰레드별로 할당합니다.
	// id를 1을 부여하였고 최대 8개의 쓰레드에서 각각의 객체 변수를 사용합니다.
	struct MyObj{
		std::string _v;

		MyObj(){
			_v = "empty value";
		}
		MyObj(const std::string &v){
			_v = v;
		}

		std::string get(){
			return _v;
		}
	};

	using obj_type = MyObj;
	const static int ref_id = 1;
	const static int capacity = 8;
	using my_ref_type = ThreadStaticValue<obj_type, ref_id, capacity>;

	// 값 출력 함수 정의
	auto valuePrint = [](){
		std::cout << "변수값 : " << my_ref_type::get().get() << std::endl;
	};

	// 값 변경 함수 정의
	auto valueChangeFunction = [](const std::string &v){
		my_ref_type::get() = obj_type(v);
	};

	obj_type &ref_a = my_ref_type::get();
	std::cout << "변수의 주소 : " << std::hex << &ref_a << std::endl;
	std::cout << ref_a.get() << std::endl;
	valueChangeFunction("A");
	valuePrint();

	valueChangeFunction("B");
	valuePrint();

	std::thread([&](){
		std::cout << "다른 쓰레드에서 이 기능을 사용해 보겠습니다." << std::endl;

		obj_type &ref_a = my_ref_type::get();
		std::cout << "변수의 주소 : " << std::hex << &ref_a << std::endl;
		std::cout << ref_a.get() << std::endl;
		valueChangeFunction("D");
		valuePrint();

	}).join();

	std::cout << std::endl << std::endl << std::endl;

	return 0;
}
*/