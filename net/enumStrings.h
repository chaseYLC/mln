#ifndef ENUMS_H
#define ENUMS_H

#include <boost/preprocessor.hpp>                                                
#include <string>

/*
 * The following macros allow you to define an enum and get `toString`
 * and `fromString` functions generated automatically.
 * If you use
 * @code
 * ENUM_WITH_STRING_CONVERSION(Test, (One)(Two)(Three))
 * @endcode
 * your enum will be called `Test` and the conversion functions will be
 * called `Test_toString` and `Test_fromString`
 */

#define X_ENUM_WITH_STRING_CONVERSION_TOSTRING_CASE(r, data, elem)   \
    case elem : return BOOST_PP_STRINGIZE(elem);                               

#define X_ENUM_WITH_STRING_CONVERSION_FROMSTRING_CASE(r, data, elem) \
    if (str == BOOST_PP_STRINGIZE(elem)) return elem;                          

#define ENUM_WITH_STRING_CONVERSION(name, enumerators)               \
    enum name {                                                      \
        Invalid,                                                     \
        BOOST_PP_SEQ_ENUM(enumerators)                               \
    };                                                               \
                                                                     \
    inline name name##_fromString(const std::string & str)           \
    {                                                                \
        BOOST_PP_SEQ_FOR_EACH(                                       \
            X_ENUM_WITH_STRING_CONVERSION_FROMSTRING_CASE,           \
            name,                                                    \
            enumerators                                              \
        )                                                            \
        return Invalid;                                              \
    }                                                                \
                                                                     \
    inline std::string name##_toString(name v)                       \
    {                                                                \
        switch (v)                                                   \
        {                                                            \
            BOOST_PP_SEQ_FOR_EACH(                                   \
                X_ENUM_WITH_STRING_CONVERSION_TOSTRING_CASE,         \
                name,                                                \
                enumerators                                          \
            )                                                        \
            default:                                                 \
                return "Invalid";                                    \
        }                                                            \
    }                                                                \

#endif /* end of include guard: ENUMS_H */