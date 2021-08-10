#pragma once
#include <tuple>
#include <cmath>
#include <numeric>

/*
  Fast way to add all comparison operators for custom struct.

  All floating-point numbers already handled.

  Global comparison operators are re-used for all types in struct,
  but any of them could be overriden for your custom struct context.


    Example
  -----------

  struct My_Struct
  {
      int a_;
      double b_;
      char* s_;

      auto tie() const { return std::tie(a_, b_, s_); }
  };


  // Custom comparer example:  strcmp instead of raw pointers comparison.
  //                           Notice. It's non-global, but only for My_Struct.
  inline int compare(const My_Struct&, const char* a, const char* b)
  {
      return strcmp(a, b);
  }

  inline bool compare_eq(const My_Struct&, const char* a, const char* b)
  {
      return strcmp(a, b) == 0;
  }


  STRUCT_COMPARISONS2(My_Struct);    // All:  <, <=, ==, !=, >, >=
  STRUCT_COMPARISONS2_EQ(My_Struct); // Only: ==, !=
*/


namespace struct_ops_internal {

template<typename Struct, typename T>
typename std::enable_if<!std::is_floating_point<T>::value, int>::type
inline compare(const Struct&, T a, T b)
{
    return (a > b) ? 1 :
           (a < b) ? -1 :
                     0;
}

/*
template<typename Struct>
inline int compare(const Struct&, const char* a, const char* b)
{
    return (a == b) ? 0 : strcmp(a, b);
}
*/

template <typename Struct, typename T>
typename std::enable_if<std::is_floating_point<T>::value, int>::type
inline compare(const Struct&, T a, T b)
{
    if (std::fabs(a - b) <= std::numeric_limits<T>::epsilon()) {
        return 0;
    } else {
        return (a > b) ? 1 : -1;
    }
}

template<typename Struct, std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), int>::type
compare(const Struct&, const std::tuple<Tp...>&, const std::tuple<Tp...>&)
{
    return 0;
}

template<typename Struct, std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), int>::type
compare(const Struct& s, const std::tuple<Tp...>& t1, const std::tuple<Tp...>& t2)
{
    auto result = compare(s, std::get<I>(t1), std::get<I>(t2));

    return result ? result :
                    compare<Struct, I + 1, Tp...>(s, t1, t2);
}

template<typename Struct, typename... Tp>
inline int compareTuples(const Struct& s, const std::tuple<Tp...>& lhs, const std::tuple<Tp...>& rhs)
{
    return compare(s, lhs, rhs);
}

//
// Eq-only
//

template<typename Struct, typename T>
typename std::enable_if<!std::is_floating_point<T>::value, bool>::type
inline compare_eq(const Struct&, T a, T b)
{
    return (a == b);
}

/*
template<typename Struct>
inline bool compare_eq(const Struct&, const char* a, const char* b)
{
    return strcmp(a, b) == 0;
}
*/

template <typename Struct, typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
inline compare_eq(const Struct&, T a, T b)
{
    return (std::fabs(a - b) <= std::numeric_limits<T>::epsilon());
}

template<typename Struct, std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), bool>::type
compare_eq(const Struct&, const std::tuple<Tp...>&, const std::tuple<Tp...>&)
{
    return true;
}

template<typename Struct, std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), bool>::type
compare_eq(const Struct& s, const std::tuple<Tp...>& t1, const std::tuple<Tp...>& t2)
{
    auto result = compare_eq(s, std::get<I>(t1), std::get<I>(t2));

    return result ? compare_eq<Struct, I + 1, Tp...>(s, t1, t2) :
                    result;
}

template<typename Struct, typename... Tp>
inline bool compareTuplesEq(const Struct& s, const std::tuple<Tp...>& lhs, const std::tuple<Tp...>& rhs)
{
    return compare_eq(s, lhs, rhs);
}

} // namespace struct_ops_internal

#define STRUCT_OP2(STRUCT, OP) \
    inline bool operator OP(const STRUCT& lhs, const STRUCT& rhs) \
    { \
        const auto result = struct_ops_internal::compareTuples(lhs, lhs.tie(), rhs.tie()); \
        return (result OP 0); \
    }

#define STRUCT_OP2_EQ(STRUCT, OP) \
    inline bool operator OP(const STRUCT& lhs, const STRUCT& rhs) \
    { \
        const auto result = struct_ops_internal::compareTuplesEq(lhs, lhs.tie(), rhs.tie()); \
        return (result OP true); \
    }

#define STRUCT_COMPARISONS2(STRUCT) \
    STRUCT_OP2(STRUCT, ==) \
    STRUCT_OP2(STRUCT, !=) \
    STRUCT_OP2(STRUCT, <) \
    STRUCT_OP2(STRUCT, <=) \
    STRUCT_OP2(STRUCT, >) \
    STRUCT_OP2(STRUCT, >=)

#define STRUCT_COMPARISONS2_ONLY_EQ(STRUCT) \
    STRUCT_OP2_EQ(STRUCT, ==) \
    STRUCT_OP2_EQ(STRUCT, !=)
