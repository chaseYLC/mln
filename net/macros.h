#pragma once

#define IS_NULL_RETURN( p, r )		if( p == 0 ) return r;

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __MEMO__ __FILE__ "("__STR1__(__LINE__)") : ♨ User-Memo >> "

#define MLN_VALUE_TYPE(templateVariable)	std::decay<decltype(templateVariable)>::type::value_type


namespace MLN{
	namespace Net {
		template<typename T>
		void safe_delete(T*& a) {
			if (nullptr != a) {
				delete a;
				a = nullptr;
			}
		}
	}
}//namespace MLN{



 /**
 @def	COUNTOF(pointer indicate to array)
 @detail	배열의 크기를 얻어온다. 배열을 가리키는 포인터가 아니라면 컴파일 오류를 일으키게한다.
 */
#define MLN_COUNTOF(x)  (                                           \
  0 * sizeof( reinterpret_cast<const ::Bad_arg_to_COUNTOF*>(x) ) +  \
  0 * sizeof( ::Bad_arg_to_COUNTOF::check_type((x), &(x))      ) +  \
  sizeof(x) / sizeof((x)[0])  )                                  

class Bad_arg_to_COUNTOF
{
public:
	class Is_pointer;  // intentionally incomplete type
	class Is_array {};
	template<typename T>
	static Is_pointer check_type(const T*, const T* const*);
	static Is_array check_type(const void*, const void*);
};
