#pragma once

#define IS_NULL_RETURN( p, r )		if( p == 0 ) return r;

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __MEMO__ __FILE__ "("__STR1__(__LINE__)") : ขอ User-Memo >> "

#define DY_VALUE_TYPE(templateVariable)	std::decay<decltype(templateVariable)>::type::value_type


namespace mln{
	template<typename T>
	void safe_delete(T*& a) {
		if (nullptr != a) {
			delete a;
			a = nullptr;
		}
	}	
}//namespace mln



 /**
 @def	COUNTOF(pointer indicate to array)
 */
#define XRNET_COUNTOF(x)  (                                           \
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
