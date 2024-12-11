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
 *  --- TOOLS ---
 *  - containerize (begin, end) - creates container-like object from iterators to use in other functions.
 *
 *
 *  --- SEARCH ---
 *
 *  There are functions, which find value in container. By default copy of value is stored internally.
 *   - find        (container, value)
 *   - find_if     (container, predicate)
 *   - find_in_map (map-container, key)
 *
 *    There are additional modifications of find functions:
 *      _cref -  internally stores reference to found item instead of copying
 *      _ref  -  same as above, but allows inplace modification
 *
 *    Returned object is similar to `std::optional`, but also has `index` method:
 *     - operator  bool() const
 *     - T&        value()            -- throws
 *     - T&        value_or_assert()  -- assert
 *     - T&        operator*          -- assert
 *     - T*        operator->         -- assert
 *     - bool      has_value() const
 *     - T         value_or(const T& alternative) const
 *     - size_t    index() const  -- unavailable for set and map containers!
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
 *   - all_of,    any_of,    none_of    (container, value,     default result if empty)
 *   - all_of_if, any_of_if, none_of_if (container, predicate, default result if empty)
 *
 *
 *   --- COPY/MODIFICATION ---
 *
 *   - copy_if   (container, predicate)
 *   - transform (container, transformer)
 *   - copy_if_transform (container, predicate, transformer)
 *
 *   - erase_all_vec (container, value-or-predicate) - shift elements and erase tail
 *   - erase_all     (container, value-or-predicate) - erase each element separately
 *   - erase_one     (container, value-or-predicate) - erase single element
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
 *
 *
 *   --- DIFFERENCE ---
 *   - difference_sorted(container1, container2)
 *     difference_sorted(container1, container2, compare)
 *     Returns: container1 - container2. Operates on sorted containers only (e.g.: std::set, sorted std::vector).
 *
 *   - difference_sorted_detailed(container1, container2, addedHandler, removedHandler)
 *     difference_sorted_detailed(container1, container2, addedHandler, removedHandler, compare)
 *
 *        AddedHandler:   void (auto srcItBegin, auto srcItEnd, int64_t insertionIndex)
 *        RemovedHandler: void (int64_t minIndex, int64_t maxIndex)
 *        (To be more precise, `container1::difference_type` is used instead of `int64_t`)
 *
 *     Behavior: this function calls `addedHandler` and `removedHandler` so container1 could be "upgraded"
 *     to container2 by provided data. First `addedHandler`s are called, then `removedHandler`s.
*/

template <class Key, class T> class QMap;
template <class T> class QSet;
template <class Key, class T> class QMultiMap;
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
    explicit operator bool() const { return has_value(); }

    const UT& value() const { return m_data.value(); }
    const UT& value_or_assert() const { assert(m_data); return *m_data; }

    template<bool rw = rw_, typename std::enable_if_t<rw>* = nullptr>
    UT& value() { return m_data.value(); }

    template<bool rw = rw_, typename std::enable_if_t<rw>* = nullptr>
    UT& value_or_assert() { assert(m_data); return *m_data; }

    UT value_or(const UT& altValue) const { return m_data.value_or(altValue); }

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

    size_t index() const { return static_cast<const PartBase<T, UT, rw_>*>(this)->value(), m_index; }

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

    template<bool rw = rw_, typename std::enable_if_t<rw>* = nullptr> UT& operator* () { return Base::value(); }
    template<bool rw = rw_, typename std::enable_if_t<rw>* = nullptr> UT* operator-> () { return &Base::value(); }
    template<bool rw = rw_, typename std::enable_if_t<rw>* = nullptr> UT& value() { return Base::value(); }

    const UT& operator* () const { return Base::value_or_assert(); }
    const UT* operator-> () const { return &Base::value_or_assert(); }
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

// Methods checking
namespace CU_Methods {

template <typename, typename = void>
struct has_reserve : std::false_type {};

template <typename T>
struct has_reserve<T, std::void_t<decltype(std::declval<T>().reserve(1))>> : std::true_type {};

template <typename, typename = void>
struct has_push_back : std::false_type {};

template <typename T>
struct has_push_back<T, std::void_t<decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))>> : std::true_type {};

template <typename, typename = void>
struct has_insert : std::false_type {};

template <typename T>
struct has_insert<T, std::void_t<decltype(std::declval<T>().insert(std::declval<typename T::value_type>()))>> : std::true_type {};

} // namespace CU_Methods

// Qt-compatibility
template<typename T>       struct IsQtAssocContainer : std::false_type {};
template<typename... Args> struct IsQtAssocContainer<QMap<Args...>> : std::true_type {};
template<typename... Args> struct IsQtAssocContainer<QMultiMap<Args...>> : std::true_type {};
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

template <template<typename...> class C, typename T, typename... Args>
class SetInserter
{
public:
    using Container = C<T, Args...>;
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = void;
    using pointer = void;
    using reference = void;

    explicit SetInserter(Container& set) : m_set(set) {}
    SetInserter(const SetInserter& other) : m_set(other.m_set) {}
    SetInserter& operator=(const SetInserter& other) { m_set = other.m_set; return *this; }

    SetInserter& operator=(const T& value)
    {
        m_set.insert(value);
        return *this;
    }

    SetInserter& operator*() { return *this; }
    SetInserter& operator++() { return *this; }
    SetInserter& operator++(int) { return *this; }

private:
    Container& m_set;
};

template <template <typename...> class C, typename T, typename... Args>
SetInserter(C<T, Args...>&) -> SetInserter<C, T, Args...>;

template<typename T,
         typename std::enable_if_t<CU_Methods::has_push_back<T>::value>* = nullptr>
auto getInserter(T& container) { return std::back_inserter(container); }

template<typename T,
         typename std::enable_if_t<!CU_Methods::has_push_back<T>::value && CU_Methods::has_insert<T>::value>* = nullptr>
auto getInserter(T& set) { return SetInserter(set); }

template<typename Container, bool = false>
struct ReserverImpl                  { template<typename T> static void tryReserve(Container&, T) {} };
template<typename Container>
struct ReserverImpl<Container, true> { template<typename T> static void tryReserve(Container& c, T size) { c.reserve(size); } };

template<typename Container, typename SizeT>
void tryReserve(Container& container, SizeT size)
{
    ReserverImpl<Container, CU_Methods::has_reserve<Container>::value>::tryReserve(container, size);
}

template<typename Iterator>
size_t distance(Iterator first, Iterator last)
{
    const auto delta = std::distance(first, last);
    assert(delta >= 0);
    return static_cast<size_t>(delta);
}

template<typename T, typename PredicateOrValue>
inline bool match(size_t index, const T& item, const PredicateOrValue& predicateOrValue)
{
    if constexpr (std::is_invocable_r_v<bool, PredicateOrValue, const T&> || std::is_invocable_r_v<bool, PredicateOrValue, size_t, const T&>)
        return callPredicate(index, predicateOrValue, item);
    else
        return item == predicateOrValue;
}

template<typename Iterator>
class Containerizer
{
public:
    Containerizer(Iterator begin, Iterator end): m_begin(begin), m_end(end) {}
    Containerizer(const Containerizer&) = default;

    Iterator begin() const { return m_begin; }
    Iterator end() const { return m_end; }

private:
    Iterator m_begin {};
    Iterator m_end {};
};

} // namespace Internal

// SearchResult

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

// containerize
template<typename Iterator>
auto containerize(Iterator begin, Iterator end)
{
    return Internal::Containerizer(begin, end);
}

// Regular find functions (copy stored)

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
auto find(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<RT, true>() :
                                          SearchResult<RT, true>(*it, Internal::distance(std::cbegin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable>
auto find_if(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<RT, true>() :
                                          SearchResult<RT, true>(*it, Internal::distance(std::cbegin(container), it));
}

// Regular find functions (ref stored)

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
auto find_ref(Container& container, const RT& value)
{
    auto it = std::find(std::begin(container), std::end(container), value);
    return (it == std::end(container)) ? SearchResult<std::reference_wrapper<RT>, true>() :
                                         SearchResult<std::reference_wrapper<RT>, true>(*it, Internal::distance(std::begin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable>
auto find_if_ref(Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::begin(container), std::end(container), predicate);
    return (it == std::end(container)) ? SearchResult<std::reference_wrapper<RT>, true>() :
                                         SearchResult<std::reference_wrapper<RT>, true>(*it, Internal::distance(std::begin(container), it));
}

// Regular find functions (const-ref stored)

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
auto find_cref(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable>
auto find_if_cref(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
auto find_ref(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable>
auto find_if_ref(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
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
                                          std::optional<size_t>(Internal::distance(std::cbegin(container), it));
}

template<typename Container,
         typename Callable>
std::optional<size_t> index_of_if(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? std::optional<size_t>() :
                                          std::optional<size_t>(Internal::distance(std::cbegin(container), it));
}

template<typename Container,
         typename ItemType>
bool all_of(const Container& container, const ItemType& value, bool defaultResult = true)
{
    if (container.size() == 0)
        return defaultResult;

    return std::all_of(std::cbegin(container), std::cend(container), [&value](const auto& item){ return item == value; });
}

template<typename Container,
         typename ItemType>
bool any_of(const Container& container, const ItemType& value, bool defaultResult = false)
{
    if (container.size() == 0)
        return defaultResult;

    return std::any_of(std::cbegin(container), std::cend(container), [&value](const auto& item){ return item == value; });
}

template<typename Container,
         typename ItemType>
bool none_of(const Container& container, const ItemType& value, bool defaultResult = true)
{
    if (container.size() == 0)
        return defaultResult;

    return std::none_of(std::cbegin(container), std::cend(container), [&value](const auto& item){ return item == value; });
}

template<typename Container,
         typename Callable>
bool all_of_if(const Container& container, const Callable& predicate, bool defaultResult = true)
{
    if (container.size() == 0)
        return defaultResult;

    return std::all_of(std::cbegin(container), std::cend(container), predicate);
}

template<typename Container,
         typename Callable>
bool any_of_if(const Container& container, const Callable& predicate, bool defaultResult = false)
{
    if (container.size() == 0)
        return defaultResult;

    return std::any_of(std::cbegin(container), std::cend(container), predicate);
}

template<typename Container,
         typename Callable>
bool none_of_if(const Container& container, const Callable& predicate, bool defaultResult = true)
{
    if (container.size() == 0)
        return defaultResult;

    return std::none_of(std::cbegin(container), std::cend(container), predicate);
}

template<template<typename...> class OverrideContainer = Internal::Empty,
         typename Container,
         typename Callable>
auto copy_if(const Container& container, const Callable& predicate)
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container, OverrideContainer<ElementType>>;

    ResultType result;
    auto it = Internal::getInserter(result);

    size_t index = static_cast<size_t>(-1);
    for (const auto& x : container)
        if (Internal::callPredicate(++index, predicate, x))
            *it++ = x;

    return result;
}

template<template<typename...> class OverrideContainer = Internal::Empty,
         template<typename...> class Container,
         typename... CArgs,
         typename Transformer>
auto transform(const Container<CArgs...>& container, const Transformer& transformer)
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using NewType = typename Internal::PredicateRetType<Transformer, ElementType>::type;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container<NewType>, OverrideContainer<NewType>>;

    ResultType result;
    Internal::tryReserve(result, container.size());
    auto it = Internal::getInserter(result);

    size_t index = static_cast<size_t>(-1);
    for (const auto& x : container)
        *it++ = Internal::callPredicate(++index, transformer, x);

    return result;
}

template<typename ResultingContainer, typename Container, typename Transformer>
auto transform(const Container& container, const Transformer& transformer)
{
    ResultingContainer result;
    Internal::tryReserve(result, container.size());
    auto it = Internal::getInserter(result);

    size_t index = static_cast<size_t>(-1);
    for (const auto& x : container)
        *it++ = Internal::callPredicate(++index, transformer, x);

    return result;
}

template<template<typename...> class OverrideContainer = Internal::Empty,
         template<typename...> class Container,
         typename... CArgs,
         typename Predicate, typename Transformer>
auto copy_if_transform(const Container<CArgs...>& container, const Predicate& predicate, const Transformer& transformer)
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using NewType = typename Internal::PredicateRetType<Transformer, ElementType>::type;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container<NewType>, OverrideContainer<NewType>>;

    ResultType result;
    auto it = Internal::getInserter(result);

    size_t index = static_cast<size_t>(-1);
    for (const auto& x : container) {
        if (Internal::callPredicate(++index, predicate, x))
            *it++ = Internal::callPredicate(index, transformer, x);
    }

    return result;
}

template<typename ResultingContainer, typename Container, typename Predicate, typename Transformer>
auto copy_if_transform(const Container& container, const Predicate& predicate, const Transformer& transformer)
{
    ResultingContainer result;
    auto it = Internal::getInserter(result);

    size_t index = static_cast<size_t>(-1);
    for (const auto& x : container) {
        if (Internal::callPredicate(++index, predicate, x))
            *it++ = Internal::callPredicate(index, transformer, x);
    }

    return result;
}

template<typename Container, typename PredicateOrValue>
void erase_all_vec(Container& container, const PredicateOrValue& predicateOrValue)
{
    size_t index = 0;
    auto it = std::begin(container);
    const auto end = std::end(container);

    while (it != end) {
        if (Internal::match(index, *it, predicateOrValue))
            break;

        ++it;
        ++index;
    }

    if (it == end)
        return;

    auto next = it;

    while (++it != end) {
        if (!Internal::match(++index, *it, predicateOrValue)) {
            *next = std::move(*it);
            ++next;
        }
    }

    container.erase(next, end);
}

template<typename Container, typename PredicateOrValue>
void erase_all(Container& container, const PredicateOrValue& predicateOrValue)
{
    size_t index = 0;
    auto it = std::begin(container);

    while (it != std::end(container)) {
        if (Internal::match(index, *it, predicateOrValue)) {
            it = container.erase(it);
            ++index;

        } else {
            ++it;
            ++index;
        }
    }
}

template<typename Container, typename PredicateOrValue>
void erase_one(Container& container, const PredicateOrValue& predicateOrValue)
{
    size_t index = 0;
    auto it = std::begin(container);

    while (it != std::end(container)) {
        if (Internal::match(index, *it, predicateOrValue)) {
            container.erase(it);
            break;

        } else {
            ++it;
            ++index;
        }
    }
}

template<template<typename...> class OverrideContainer = Internal::Empty,
         typename Compare = std::less<>,
         template<typename...> class Container,
         typename... CArgs,
         typename AnotherContainer>
auto difference_sorted(const Container<CArgs...>& container1, const AnotherContainer& container2, const Compare& compare = {})
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container1))>>;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container<CArgs...>, OverrideContainer<ElementType>>;

    assert(std::is_sorted(std::cbegin(container1), std::cend(container1), compare));
    assert(std::is_sorted(std::cbegin(container2), std::cend(container2), compare));

    ResultType result;
    std::set_difference(std::cbegin(container1), std::cend(container1),
                        std::cbegin(container2), std::cend(container2),
                        Internal::getInserter(result), compare);

    return result;
}

template<typename Compare = std::less<>,
         typename AddedHandler, typename RemovedHandler,
         typename Container1, typename Container2>
void difference_sorted_detailed(const Container1& container1, const Container2& container2,
                                const AddedHandler& addedHandler, const RemovedHandler& removedHandler,
                                const Compare& compare = {})
{
    assert(std::is_sorted(std::cbegin(container1), std::cend(container1), compare));
    assert(std::is_sorted(std::cbegin(container2), std::cend(container2), compare));

    using DiffType = typename Container1::difference_type;

    auto it1 = std::cbegin(container1);
    auto end1 = std::cend(container1);
    auto it2 = std::cbegin(container2);
    auto end2 = std::cend(container2);

    DiffType index1 = 0;
    DiffType offset = 0;

    while (it1 != end1 || it2 != end2) {
        if (it1 == end1) {
            // All remaining elements in container2 are additions
            if (it2 != end2) {
                DiffType insertionIndex = index1 + offset;
                auto itemsStart = it2;
                DiffType count = std::distance(it2, end2);
                addedHandler(itemsStart, end2, insertionIndex);
                offset += count;
            }
            break;
        }

        if (it2 == end2) {
            // All remaining elements in container1 are removals
            if (it1 != end1) {
                DiffType minIndex = index1 + offset;
                DiffType count = std::distance(it1, end1);
                DiffType maxIndex = minIndex + count - 1;
                removedHandler(minIndex, maxIndex);
                offset -= count;
            }
            break;
        }

        if (!compare(*it1, *it2) && !compare(*it2, *it1)) {
            // Elements are equal
            ++it1;
            ++it2;
            ++index1;
        } else if (compare(*it1, *it2)) {
            // Element is in container1 but not in container2 (removal)
            DiffType minIndex = index1 + offset;
            DiffType removalCount = 0;

            while (it1 != end1 && (it2 == end2 || compare(*it1, *it2))) {
                ++it1;
                ++index1;
                ++removalCount;
            }

            DiffType maxIndex = minIndex + removalCount - 1;
            removedHandler(minIndex, maxIndex);
            offset -= removalCount;
        } else {
            // Element is in container2 but not in container1 (addition)
            DiffType insertionIndex = index1 + offset;
            auto itemsStart = it2;
            DiffType additionCount = 0;

            while (it2 != end2 && (it1 == end1 || compare(*it2, *it1))) {
                ++it2;
                ++additionCount;
            }

            addedHandler(itemsStart, it2, insertionIndex);
            offset += additionCount;
        }
    }
}

} // namespace utils_cpp
