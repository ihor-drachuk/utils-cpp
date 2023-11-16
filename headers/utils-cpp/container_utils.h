/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <optional>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>
#include <cassert>


/*  Overview
 *
 *  --- SEARCH ---
 *
 *  There are functions, which find value in container. By default copy of value is stored internally.
 *   - find        (container, value)
 *   - find_if     (container, predicate)
 *   - find_in_map (map-container, key)
 *  There are additional modifications of find functions:
 *    _cref -  internally stores reference to found item instead of copying
 *    _ref  -  same as above, but allows inplace modification
 *
 *  Also:
 *   - contains     (container, value)
 *   - contains_if  (container, predicate)
 *   - contains_set (set-container, value)
 *   - contains_map (map-container, key)
 *
 *   - index_of     (container, value)
 *   - index_of_if  (container, predicate)
 *
 *  Returned object is similar to `std::optional`, but also has `index` method:
 *   - operator  bool() const
 *   - T&        value()
 *   - T&        operator*
 *   - T*        operator->
 *   - bool      has_value() const
 *   - T         value_or(const T& alternative) const
 *   - size_t    index() const  -- unavailable for set and map containers!
 *
 *
 *   --- COPY/MODIFICATION ---
 *
 *   - copy_if(container, predicate)
 *   - transform(container, transformer)
 *   - copy_if_transform(container, predicate, transformer)
 *
 *   Notice:
 *    - By default, return type is same container of same types (copy_if) or
 *                                 same container of transformed types (*transform)
 *
 *    - First template parameter could be used to override returned container type.
 *      E.g.: copy_if<std::list>(someVector, somePredicate); - this will return std::list instead of vector.
 *
 *    - Predicate and transformer can have index of currently processed item. I.e. both variants are valid:
 *        [] (const auto& value) { return value.empty(); }
 *        [] (size_t index, const auto& value) { return index !== 0 && value.empty(); }
*/

template <class Key, class T> class QMap;
template <class Key, class T> class QHash;

namespace utils_cpp {

namespace Internal {

template<typename>   struct Empty {};

template<typename>   struct IsAnyRef                            : std::false_type {};
template<typename T> struct IsAnyRef<std::reference_wrapper<T>> : std::true_type {};
template<typename>   struct IsCRef                                  : std::false_type {};
template<typename T> struct IsCRef<std::reference_wrapper<const T>> : std::true_type {};

template<typename T> struct Unwrap                            { using type = T; };
template<typename T> struct Unwrap<std::reference_wrapper<T>> { using type = std::remove_const_t<T>; };

template<typename T, typename UT, bool rw_>
class PartBase
{
public:
    PartBase() = default;
    PartBase(const T& data): m_data(data) {}

    bool has_value() const { return m_data.has_value(); }
    explicit operator bool() const { return has_value(); };

    const UT& value() const { return m_data.value(); }

    template<bool rw = rw_, typename std::enable_if_t<rw>* = nullptr>
    UT& value() { return m_data.value(); }

    UT value_or(const UT& altValue) const { return has_value() ? value() : altValue; }

    operator std::optional<UT>() const { return has_value() ? std::optional<UT>(value()) : std::optional<UT>(); }

private:
    std::optional<T> m_data;
};


template<typename T, typename UT, bool rw_>
class PartIndex : public PartBase<T, UT, rw_>
{
public:
    PartIndex() = default;
    PartIndex(const T& data, size_t index): PartBase<T, UT, rw_>(data), m_index(index) {}

    size_t index() const { return static_cast<const PartBase<T, UT, rw_>*>(this)->value(), m_index; };

private:
    size_t m_index{};
};

template<typename T, typename UT, bool rw_, bool needIndex_,
         typename Base = std::conditional_t<needIndex_,
                                            PartIndex<T, UT, rw_>,
                                            PartBase<T, UT, rw_>>>
class PartOperators : public Base
{
public:
    PartOperators() = default;

    template<bool needIndex = needIndex_, typename std::enable_if_t<needIndex>* = nullptr>
    PartOperators(const T& data, size_t index): Base(data, index) {}

    template<bool needIndex = needIndex_, typename std::enable_if_t<!needIndex>* = nullptr>
    PartOperators(const T& data): Base(data) {}

    template<bool rw = rw_, typename std::enable_if_t<rw>* = nullptr> UT& operator* () { return Base::value(); };
    template<bool rw = rw_, typename std::enable_if_t<rw>* = nullptr> UT* operator-> () { return &Base::value(); }
    template<bool rw = rw_, typename std::enable_if_t<rw>* = nullptr> UT& value() { return Base::value(); }

    const UT& operator* () const { return Base::value(); };
    const UT* operator-> () const { return &Base::value(); }
    const UT& value() const { return Base::value(); }
};


template<bool, typename, typename> struct PredicateCaller { };

template<typename Predicate, typename Item>
struct PredicateCaller<true, Predicate, Item> // With index
{
    using RetType = decltype(std::declval<Predicate>()(size_t(), std::declval<Item>()));
    static auto call(size_t index, const Predicate& predicate, const Item& item) { return predicate(index, item); }
};

template<typename Predicate, typename Item>
struct PredicateCaller<false, Predicate, Item> // W/o index
{
    using RetType = decltype(std::declval<Predicate>()(std::declval<Item>()));
    static auto call(size_t, const Predicate& predicate, const Item& item) { return predicate(item); }
};

template<typename Predicate, typename Item>
struct PredicateCallerSel : public PredicateCaller<std::is_invocable_v<Predicate, size_t, Item>, Predicate, Item> { };

template<typename Predicate, typename Item>
auto callPredicate(size_t index, const Predicate& predicate, const Item& item)
{
    return PredicateCallerSel<Predicate, Item>::call(index, predicate, item);
}

template<typename Predicate, typename Item>
struct PredicateRetType
{
    using type = typename PredicateCallerSel<Predicate, Item>::RetType;
};

// Qt-compatibility
template<typename T>       struct IsQtAssocContainer : std::false_type {};
template<typename... Args> struct IsQtAssocContainer<QMap<Args...>> : std::true_type {};
template<typename... Args> struct IsQtAssocContainer<QHash<Args...>> : std::true_type {};

struct MapHelperStl
{
    template<typename Iter> static  auto        value(Iter it)     { return it->second; }
    template<typename Iter> static  const auto& valueCref(Iter it) { return it->second; }
    template<typename Iter> static  auto&       valueRef(Iter it)  { return it->second; }
};

struct MapHelperQt
{
    template<typename Iter> static  auto        value(Iter it)     { return *it; }
    template<typename Iter> static  const auto& valueCref(Iter it) { return *it; }
    template<typename Iter> static  auto&       valueRef(Iter it)  { return *it; }
};

template<typename T>
struct MapHelper : std::conditional_t<IsQtAssocContainer<T>::value, MapHelperQt, MapHelperStl> {};

} // namespace Internal


template<typename T,
         bool needIndex_,
         typename UT = typename Internal::Unwrap<T>::type,
         bool rw = Internal::IsAnyRef<T>::value && !Internal::IsCRef<T>::value
         >
class SearchResult : public Internal::PartOperators<T, UT, rw, needIndex_>
{
public:
    SearchResult() = default;

    template<bool needIndex = needIndex_, typename std::enable_if_t<needIndex>* = nullptr>
    SearchResult(const T& data, size_t index): Internal::PartOperators<T, UT, rw, needIndex_>(data, index) {}

    template<bool needIndex = needIndex_, typename std::enable_if_t<!needIndex>* = nullptr>
    SearchResult(const T& data): Internal::PartOperators<T, UT, rw, needIndex_>(data) {}
};

// Regular find functions (copy stored)

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
auto find(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<RT, true>() :
                                          SearchResult<RT, true>(*it, std::distance(std::cbegin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable>
auto find_if(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<RT, true>() :
                                          SearchResult<RT, true>(*it, std::distance(std::cbegin(container), it));
}

// Regular find functions (ref stored)

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
auto find_ref(Container& container, const RT& value)
{
    auto it = std::find(std::begin(container), std::end(container), value);
    return (it == std::end(container)) ? SearchResult<std::reference_wrapper<RT>, true>() :
                                         SearchResult<std::reference_wrapper<RT>, true>(*it, std::distance(std::begin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable>
auto find_if_ref(Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::begin(container), std::end(container), predicate);
    return (it == std::end(container)) ? SearchResult<std::reference_wrapper<RT>, true>() :
                                         SearchResult<std::reference_wrapper<RT>, true>(*it, std::distance(std::begin(container), it));
}

// Regular find functions (const-ref stored)

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
auto find_cref(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, std::distance(std::cbegin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable>
auto find_if_cref(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, std::distance(std::cbegin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
auto find_ref(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, std::distance(std::cbegin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable>
auto find_if_ref(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, std::distance(std::cbegin(container), it));
}

// Find in map

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::mapped_type, T>,
         typename KT>
std::optional<RT> find_in_map(const Container& container, const KT& key)
{
    auto it = container.find(key);
    return (it == std::cend(container)) ? std::optional<RT>() :
                                          std::optional<RT>(Internal::MapHelper<Container>::value(it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::mapped_type, T>,
         typename KT>
auto find_in_map_ref(Container& container, const KT& key)
{
    auto it = container.find(key);
    return (it == std::end(container)) ? SearchResult<std::reference_wrapper<RT>, false>() :
                                         SearchResult<std::reference_wrapper<RT>, false>(Internal::MapHelper<Container>::valueRef(it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::mapped_type, T>,
         typename KT>
auto find_in_map_ref(const Container& container, const KT& key)
{
    auto it = container.find(key);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, false>() :
                                          SearchResult<std::reference_wrapper<const RT>, false>(Internal::MapHelper<Container>::valueCref(it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::mapped_type, T>,
         typename KT>
auto find_in_map_cref(const Container& container, const KT& key)
{
    auto it = container.find(key);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, false>() :
                                          SearchResult<std::reference_wrapper<const RT>, false>(Internal::MapHelper<Container>::valueCref(it));
}

// contains

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
bool contains(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it != std::cend(container));
}

template<typename Container,
         typename Callable>
bool contains_if(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it != std::cend(container));
}

template<typename Container,
         typename KT>
bool contains_set(const Container& container, const KT& value)
{
    auto it = container.find(value);
    return (it != std::cend(container));
}

template<typename Container,
         typename KT>
bool contains_map(const Container& container, const KT& key)
{
    auto it = container.find(key);
    return (it != std::cend(container));
}

// index_of

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
std::optional<size_t> index_of(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? std::optional<size_t>() :
                                          std::optional<size_t>(std::distance(std::cbegin(container), it));
}

template<typename Container,
         typename Callable>
std::optional<size_t> index_of_if(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? std::optional<size_t>() :
                                          std::optional<size_t>(std::distance(std::cbegin(container), it));
}

template<template<typename...> class OverrideContainer = Internal::Empty,
         typename Container,
         typename Callable>
auto copy_if(const Container& container, const Callable& predicate)
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container, OverrideContainer<ElementType>>;

    ResultType result;
    auto it = std::back_inserter(result);

    size_t index = -1;
    for (const auto& x : container)
        if (Internal::callPredicate(++index, predicate, x))
            *it++ = x;

    return result;
}

template<template<typename...> class OverrideContainer = Internal::Empty,
         template<typename...> class Container,
         typename CArg0, typename... CArgs,
         typename Transformer>
auto transform(const Container<CArg0, CArgs...>& container, const Transformer& transformer)
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using NewType = typename Internal::PredicateRetType<Transformer, ElementType>::type;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container<NewType>, OverrideContainer<NewType>>;

    ResultType result;
    auto it = std::back_inserter(result);

    size_t index = -1;
    for (const auto& x : container)
        *it++ = Internal::callPredicate(++index, transformer, x);

    return result;
}

template<template<typename...> class OverrideContainer = Internal::Empty,
         template<typename...> class Container,
         typename CArg0, typename... CArgs,
         typename Predicate, typename Transformer>
auto copy_if_transform(const Container<CArg0, CArgs...>& container, const Predicate& predicate, const Transformer& transformer)
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using NewType = typename Internal::PredicateRetType<Transformer, ElementType>::type;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container<NewType>, OverrideContainer<NewType>>;

    ResultType result;
    auto it = std::back_inserter(result);

    size_t index = -1;
    for (const auto& x : container) {
        if (Internal::callPredicate(++index, predicate, x))
            *it++ = Internal::callPredicate(index, transformer, x);
    }

    return result;
}

} // namespace utils_cpp
