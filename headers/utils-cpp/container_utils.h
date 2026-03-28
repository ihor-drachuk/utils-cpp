/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>
#include <optional>
#include <type_traits>
#include <utility>

#if __cplusplus >= 201703L && defined(__cpp_lib_parallel_algorithm) && defined(UTILS_CPP_EXECUTION_POLICY_ENABLED)
#include <execution>
#define UTILS_CPP_HAS_EXECUTION_POLICIES 1
#endif

// For random stuff
#include <random>
#include <limits>
#include <vector>


/*  Overview
 *
 *  Note: Many functions support C++17 ExecutionPolicy (std::execution::seq, std::execution::par, etc.)
 *        as an optional first parameter for parallel execution.
 *
 *  --- TOOLS ---
 *
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
 *   - count    (container, value)
 *   - count_if (container, predicate)
 *
 *   - accumulate (container, init = T(), op = std::plus<>, defaultValue = std::nullopt)
 *   - reduce     (container, init = T(), op = std::plus<>, defaultValue = std::nullopt)
 *   - transform_reduce (container, init, reduce_op, transform_op, defaultValue = std::nullopt)
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
 *
 *   - difference_sorted(container1, container2, compare = {})
 *
 *     Returns `container1 - container2`. Operates on sorted containers only (e.g.: std::set, sorted std::vector).
 *     Sorting function/functor can be provided via template argument or `compare` parameter.
 *
 *   - difference_sorted_detailed(container1, container2,
 *                                addedHandler, removedHandler, changedHandler = nullptr,
 *                                enableCompareEq = false, compareLess = {}, compareEq = {})
 *
 *     Calls `addedHandler`, `removedHandler`, and optionally `changedHandler` so `container1` can be "upgraded"
 *     to `container2` via parameters in handlers. Operates on sorted containers only.
 *
 *     Handlers signature:
 *       AddedHandler:   void (auto srcItBegin2, auto srcItEnd2, auto dstInsertionIndex)
 *       RemovedHandler: void (auto dstMinIndex, auto dstMaxIndex)
 *       ChangedHandler: void (auto srcItBegin2, auto srcItEnd2, auto dstStartIndex)
 *
 *     IMPORTANT: Handlers must NOT modify `container1` or `container2` during the comparison process.
 *     The function holds iterators to both containers, and any modification would invalidate them.
 *     All modifications should be applied to a separate working copy, which can then replace `container1`.
 *
 *     When `enableCompareEq` is true and `changedHandler` is provided, the function detects when elements
 *     are sort-equal (according to compareLess) but not truly equal (according to compareEq) and calls
 *     the changedHandler for such elements.
 *
 *     It's expected from user to apply operations to a working copy: copy items from [srcItBegin2; srcItEnd2)
 *     to `dstInsertionIndex` position, remove items within [dstMinIndex; dstMaxIndex] range, and update changed
 *     items at `dstStartIndex` onwards using data from [srcItBegin2; srcItEnd2).
 *
 *     To be more precise:
 *         AddedHandler:   void (Container2::iterator         srcItBegin2,
 *                               Container2::iterator         srcItEnd2,
 *                               Container2::difference_type  dstInsertionIndex)
 *
 *         RemovedHandler: void (Container2::difference_type  dstMinIndex,
 *                               Container2::difference_type  dstMaxIndex)
 *
 *         ChangedHandler: void (Container2::iterator         srcItBegin2,
 *                               Container2::iterator         srcItEnd2,
 *                               Container2::difference_type  dstStartIndex)
 *
 *
 *   --- SELECTION ---
 *
 *   - random_item                   (container)
 *   - random_items       <Container>(container, count)
 *   - random_items_unique<Container>(container, count)
 *
 *   - random_weighted_item                   (container,        weightPredicate)
 *   - random_weighted_items       <Container>(container, count, weightPredicate)
 *   - random_weighted_items_unique<Container>(container, count, weightPredicate)
 *
 *
 *   --- GENERATION ---
 *
 *   - generate    <Container>(count, generator)
 *   - generate_rnd<Container>(count, generator) // Random 'size_t' is passed to 'generator'
 *   - generate_rnd<Container>(count, min, max, generator = {})
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
    size_t size() const { return static_cast<size_t>(distance(m_begin, m_end)); }

private:
    Iterator m_begin {};
    Iterator m_end {};
};

template<typename T>
T random(T min, T max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dis(min, max);
    return dis(gen);
}

template<typename T>
T random(T max)
{
    return random(T(), max);
}

template<typename T>
class DefaultGenerator
{
public:
    T operator()(const T& x) const { return x; }
};

template<typename NewArg0, typename T>
struct ReplaceArg0
{
    using Type = T;
};

template<typename NewArg0, template<typename...> class C, typename... Args>
struct ReplaceArg0<NewArg0, C<Args...>>
{
    using Type = C<NewArg0>;
};

template<typename Container, typename OffsetT>
const auto& at(const Container& c, OffsetT offset)
{
    return *(std::cbegin(c) + static_cast<typename Container::difference_type>(offset));
}

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
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
auto find(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<RT, true>() :
                                          SearchResult<RT, true>(*it, Internal::distance(std::cbegin(container), it));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
auto find(ExecutionPolicy&& policy, const Container& container, const RT& value)
{
    auto it = std::find(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<RT, true>() :
                                          SearchResult<RT, true>(*it, Internal::distance(std::cbegin(container), it));
}
#endif

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
auto find_if(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<RT, true>() :
                                          SearchResult<RT, true>(*it, Internal::distance(std::cbegin(container), it));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
auto find_if(ExecutionPolicy&& policy, const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<RT, true>() :
                                          SearchResult<RT, true>(*it, Internal::distance(std::cbegin(container), it));
}
#endif

// Regular find functions (ref stored)

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
auto find_ref(Container& container, const RT& value)
{
    auto it = std::find(std::begin(container), std::end(container), value);
    return (it == std::end(container)) ? SearchResult<std::reference_wrapper<RT>, true>() :
                                         SearchResult<std::reference_wrapper<RT>, true>(*it, Internal::distance(std::begin(container), it));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
auto find_ref(ExecutionPolicy&& policy, Container& container, const RT& value)
{
    auto it = std::find(std::forward<ExecutionPolicy>(policy), std::begin(container), std::end(container), value);
    return (it == std::end(container)) ? SearchResult<std::reference_wrapper<RT>, true>() :
                                         SearchResult<std::reference_wrapper<RT>, true>(*it, Internal::distance(std::begin(container), it));
}
#endif

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
auto find_if_ref(Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::begin(container), std::end(container), predicate);
    return (it == std::end(container)) ? SearchResult<std::reference_wrapper<RT>, true>() :
                                         SearchResult<std::reference_wrapper<RT>, true>(*it, Internal::distance(std::begin(container), it));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
auto find_if_ref(ExecutionPolicy&& policy, Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::forward<ExecutionPolicy>(policy), std::begin(container), std::end(container), predicate);
    return (it == std::end(container)) ? SearchResult<std::reference_wrapper<RT>, true>() :
                                         SearchResult<std::reference_wrapper<RT>, true>(*it, Internal::distance(std::begin(container), it));
}
#endif

// Regular find functions (const-ref stored)

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
auto find_cref(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
auto find_cref(ExecutionPolicy&& policy, const Container& container, const RT& value)
{
    auto it = std::find(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}
#endif

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
auto find_if_cref(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
auto find_if_cref(ExecutionPolicy&& policy, const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}
#endif

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
auto find_ref(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
auto find_ref(ExecutionPolicy&& policy, const Container& container, const RT& value)
{
    auto it = std::find(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}
#endif

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
auto find_if_ref(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
auto find_if_ref(ExecutionPolicy&& policy, const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<std::reference_wrapper<const RT>, true>() :
                                          SearchResult<std::reference_wrapper<const RT>, true>(*it, Internal::distance(std::cbegin(container), it));
}
#endif

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
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
bool contains(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it != std::cend(container));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
bool contains(ExecutionPolicy&& policy, const Container& container, const RT& value)
{
    auto it = std::find(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), value);
    return (it != std::cend(container));
}
#endif

template<typename Container,
         typename Callable,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
bool contains_if(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it != std::cend(container));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename Container,
         typename Callable,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
bool contains_if(ExecutionPolicy&& policy, const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), predicate);
    return (it != std::cend(container));
}
#endif

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
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
std::optional<size_t> index_of(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? std::optional<size_t>() :
                                          std::optional<size_t>(Internal::distance(std::cbegin(container), it));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
std::optional<size_t> index_of(ExecutionPolicy&& policy, const Container& container, const RT& value)
{
    auto it = std::find(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? std::optional<size_t>() :
                                          std::optional<size_t>(Internal::distance(std::cbegin(container), it));
}
#endif

template<typename Container,
         typename Callable,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
std::optional<size_t> index_of_if(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? std::optional<size_t>() :
                                          std::optional<size_t>(Internal::distance(std::cbegin(container), it));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename Container,
         typename Callable,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
std::optional<size_t> index_of_if(ExecutionPolicy&& policy, const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? std::optional<size_t>() :
                                          std::optional<size_t>(Internal::distance(std::cbegin(container), it));
}
#endif

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
         typename Callable,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
bool all_of_if(const Container& container, const Callable& predicate, bool defaultResult = true)
{
    if (container.size() == 0)
        return defaultResult;

    return std::all_of(std::cbegin(container), std::cend(container), predicate);
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename Container,
         typename Callable,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
bool all_of_if(ExecutionPolicy&& policy, const Container& container, const Callable& predicate, bool defaultResult = true)
{
    if (container.size() == 0)
        return defaultResult;

    return std::all_of(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), predicate);
}
#endif

template<typename Container,
         typename Callable,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
bool any_of_if(const Container& container, const Callable& predicate, bool defaultResult = false)
{
    if (container.size() == 0)
        return defaultResult;

    return std::any_of(std::cbegin(container), std::cend(container), predicate);
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename Container,
         typename Callable,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
bool any_of_if(ExecutionPolicy&& policy, const Container& container, const Callable& predicate, bool defaultResult = false)
{
    if (container.size() == 0)
        return defaultResult;

    return std::any_of(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), predicate);
}
#endif

template<typename Container,
         typename Callable,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
bool none_of_if(const Container& container, const Callable& predicate, bool defaultResult = true)
{
    if (container.size() == 0)
        return defaultResult;

    return std::none_of(std::cbegin(container), std::cend(container), predicate);
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename Container,
         typename Callable,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
bool none_of_if(ExecutionPolicy&& policy, const Container& container, const Callable& predicate, bool defaultResult = true)
{
    if (container.size() == 0)
        return defaultResult;

    return std::none_of(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), predicate);
}
#endif

template<typename Container,
         typename ItemType,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
size_t count(const Container& container, const ItemType& value)
{
    return static_cast<size_t>(std::count(std::cbegin(container), std::cend(container), value));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename Container,
         typename ItemType,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
size_t count(ExecutionPolicy&& policy, const Container& container, const ItemType& value)
{
    return static_cast<size_t>(std::count(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), value));
}
#endif

template<typename Container,
         typename Callable,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
size_t count_if(const Container& container, const Callable& predicate)
{
    return static_cast<size_t>(std::count_if(std::cbegin(container), std::cend(container), predicate));
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename Container,
         typename Callable,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
size_t count_if(ExecutionPolicy&& policy, const Container& container, const Callable& predicate)
{
    return static_cast<size_t>(std::count_if(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), predicate));
}
#endif

template<typename Container,
         typename T = typename Container::value_type,
         typename BinaryOp = std::plus<>,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
T accumulate(const Container& container, T init = T(), const BinaryOp& op = {}, std::optional<T> defaultValue = std::nullopt)
{
    if (container.empty())
        return defaultValue.value_or(init);

    return std::accumulate(std::cbegin(container), std::cend(container), init, op);
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename Container,
         typename T = typename Container::value_type,
         typename BinaryOp = std::plus<>,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
T accumulate(ExecutionPolicy&& policy, const Container& container, T init = T(), const BinaryOp& op = {}, std::optional<T> defaultValue = std::nullopt)
{
    if (container.empty())
        return defaultValue.value_or(init);

    return std::reduce(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), init, op);
}
#endif

template<typename Container,
         typename T = typename Container::value_type,
         typename BinaryOp = std::plus<>,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
T reduce(const Container& container, T init = T(), const BinaryOp& op = {}, std::optional<T> defaultValue = std::nullopt)
{
    if (container.empty())
        return defaultValue.value_or(init);

    return std::reduce(std::cbegin(container), std::cend(container), init, op);
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename Container,
         typename T = typename Container::value_type,
         typename BinaryOp = std::plus<>,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
T reduce(ExecutionPolicy&& policy, const Container& container, T init = T(), const BinaryOp& op = {}, std::optional<T> defaultValue = std::nullopt)
{
    if (container.empty())
        return defaultValue.value_or(init);

    return std::reduce(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), init, op);
}
#endif

template<typename Container,
         typename T,
         typename BinaryReduceOp,
         typename UnaryTransformOp,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Container>>>
#else
         typename = void
#endif
         >
T transform_reduce(const Container& container, T init, const BinaryReduceOp& reduce_op, const UnaryTransformOp& transform_op, std::optional<T> defaultValue = std::nullopt)
{
    if (container.empty())
        return defaultValue.value_or(init);

    return std::transform_reduce(std::cbegin(container), std::cend(container), init, reduce_op, transform_op);
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename Container,
         typename T,
         typename BinaryReduceOp,
         typename UnaryTransformOp,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
T transform_reduce(ExecutionPolicy&& policy, const Container& container, T init, const BinaryReduceOp& reduce_op, const UnaryTransformOp& transform_op, std::optional<T> defaultValue = std::nullopt)
{
    if (container.empty())
        return defaultValue.value_or(init);

    return std::transform_reduce(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), init, reduce_op, transform_op);
}
#endif

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
         typename Transformer,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Transformer>>>
#else
         typename = void
#endif
         >
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

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<template<typename...> class OverrideContainer = Internal::Empty,
         template<typename...> class Container,
         typename ExecutionPolicy,
         typename... CArgs,
         typename Transformer,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
auto transform(ExecutionPolicy&& policy, const Container<CArgs...>& container, const Transformer& transformer)
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using NewType = std::decay_t<decltype(transformer(std::declval<ElementType>()))>;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container<NewType>, OverrideContainer<NewType>>;

    static_assert(Internal::CU_Methods::has_push_back<ResultType>::value,
                  "ExecutionPolicy is not supported for set-like containers. Use sequential version without ExecutionPolicy.");

    ResultType result;
    Internal::tryReserve(result, container.size());
    result.resize(container.size());
    std::transform(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), std::begin(result), transformer);

    return result;
}
#endif

template<typename ResultingContainer,
         typename Container,
         typename Transformer,
#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
         typename = std::enable_if_t<!std::is_execution_policy_v<std::decay_t<ResultingContainer>>>
#else
         typename = void
#endif
         >
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

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
template<typename ExecutionPolicy,
         typename ResultingContainer,
         typename Container,
         typename Transformer,
         typename = std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
auto transform(ExecutionPolicy&& policy, const Container& container, const Transformer& transformer)
{
    static_assert(Internal::CU_Methods::has_push_back<ResultingContainer>::value,
                  "ExecutionPolicy is not supported for set-like containers. Use sequential version without ExecutionPolicy.");

    ResultingContainer result;
    Internal::tryReserve(result, container.size());
    result.resize(container.size());
    std::transform(std::forward<ExecutionPolicy>(policy), std::cbegin(container), std::cend(container), std::begin(result), transformer);

    return result;
}
#endif

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
    static_assert(!std::is_const_v<Container>);

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
    static_assert(!std::is_const_v<Container>);

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
    static_assert(!std::is_const_v<Container>);

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

template<typename CompareLess = std::less<>,
         typename CompareEq = std::equal_to<>,
         typename AddedHandler, typename RemovedHandler, typename ChangedHandler = std::nullptr_t,
         typename Container1, typename Container2>
void difference_sorted_detailed(const Container1& container1,
                                const Container2& container2,
                                const AddedHandler& addedHandler,
                                const RemovedHandler& removedHandler,
                                const ChangedHandler& changedHandler = nullptr,
                                bool enableCompareEq = false,
                                const CompareLess& compareLess = {},
                                const CompareEq& compareEq = {})
{
    assert(std::is_sorted(std::cbegin(container1), std::cend(container1), compareLess));
    assert(std::is_sorted(std::cbegin(container2), std::cend(container2), compareLess));

    using DiffType = typename Container2::difference_type;

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
                DiffType dstInsertionIndex = index1 + offset;
                auto itemsStart = it2;
                DiffType count = std::distance(it2, end2);
                addedHandler(itemsStart, end2, dstInsertionIndex);
                offset += count;
            }
            break;
        }

        if (it2 == end2) {
            // All remaining elements in container1 are removals
            if (it1 != end1) {
                DiffType dstMinIndex = index1 + offset;
                DiffType count = std::distance(it1, end1);
                DiffType dstMaxIndex = dstMinIndex + count - 1;
                removedHandler(dstMinIndex, dstMaxIndex);
                offset -= count;
            }
            break;
        }

        if (!compareLess(*it1, *it2) && !compareLess(*it2, *it1)) {
            // Elements are sort-equal
            if constexpr (!std::is_same_v<ChangedHandler, std::nullptr_t>) {
                if (enableCompareEq && !compareEq(*it1, *it2)) {
                    // Start of a changed sequence
                    DiffType dstStartIndex = index1 + offset;
                    auto srcItBegin = it2;
                    DiffType changedCount = 1;

                    auto tempIt1 = it1;
                    auto tempIt2 = it2;
                    auto tempIndex1 = index1;

                    ++tempIt1;
                    ++tempIt2;
                    ++tempIndex1;

                    // Find consecutive changed elements (must be sort-equal but not truly equal)
                    while (tempIt1 != end1 && tempIt2 != end2 &&
                           !compareLess(*tempIt1, *tempIt2) && !compareLess(*tempIt2, *tempIt1) &&
                           !compareEq(*tempIt1, *tempIt2)) {
                        ++changedCount;
                        ++tempIt1;
                        ++tempIt2;
                        ++tempIndex1;
                    }

                    // Call handler for the entire batch
                    auto srcItEnd = tempIt2;
                    changedHandler(srcItBegin, srcItEnd, dstStartIndex);

                    // Update iterators to skip the batch
                    for (DiffType i = 0; i < changedCount; ++i) {
                        ++it1;
                        ++it2;
                        ++index1;
                    }
                    continue;  // Continue to next iteration of main loop
                }
            } else {
                assert(!enableCompareEq && "ChangedHandler must be provided if enableCompareEq is true.");
            }
            ++it1;
            ++it2;
            ++index1;
        } else if (compareLess(*it1, *it2)) {
            // Element is in container1 but not in container2 (removal)
            DiffType dstMinIndex = index1 + offset;
            DiffType removalCount = 0;

            while (it1 != end1 && (it2 == end2 || compareLess(*it1, *it2))) {
                ++it1;
                ++index1;
                ++removalCount;
            }

            DiffType dstMaxIndex = dstMinIndex + removalCount - 1;
            removedHandler(dstMinIndex, dstMaxIndex);
            offset -= removalCount;
        } else {
            // Element is in container2 but not in container1 (addition)
            DiffType dstInsertionIndex = index1 + offset;
            auto itemsStart = it2;
            DiffType additionCount = 0;

            while (it2 != end2 && (it1 == end1 || compareLess(*it2, *it1))) {
                ++it2;
                ++additionCount;
            }

            addedHandler(itemsStart, it2, dstInsertionIndex);
            offset += additionCount;
        }
    }
}

template<typename Container,
         typename Generator>
auto generate(size_t count, Generator generator)
{
    using ElementType = std::decay_t<decltype(generator())>;
    using ResultType = typename Internal::ReplaceArg0<ElementType, Container>::Type;

    ResultType result;
    Internal::tryReserve(result, count);
    auto inserter = Internal::getInserter(result);

    while (count) {
        *inserter++ = generator();
        --count;
    }

    return result;
}

template<template<typename...> class Container,
         typename Generator>
auto generate(size_t count, Generator generator)
{
    return generate<Container<void>>(count, generator);
}

template<typename Container,
         typename Generator>
auto generate_rnd(size_t count, Generator generator)
{
    using ElementType = std::decay_t<decltype(generator(size_t()))>;
    using ResultType = typename Internal::ReplaceArg0<ElementType, Container>::Type;

    if (!count)
        return ResultType();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, std::numeric_limits<size_t>::max());

    ResultType result;
    Internal::tryReserve(result, count);
    auto inserter = Internal::getInserter(result);

    while (count) {
        *inserter++ = generator(dis(gen));
        --count;
    }

    return result;
}

template<template<typename...> class Container,
         typename Generator>
auto generate_rnd(size_t count, Generator generator)
{
    return generate_rnd<Container<void>>(count, generator);
}


template<typename Container,
         typename T,
         typename Generator = Internal::DefaultGenerator<T>>
auto generate_rnd(size_t count, T min, T max, Generator generator = {})
{
    using ElementType = std::decay_t<decltype(generator(size_t()))>;
    using ResultType = typename Internal::ReplaceArg0<ElementType, Container>::Type;

    if (!count)
        return ResultType();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dis(min, max);

    ResultType result;
    Internal::tryReserve(result, count);
    auto inserter = Internal::getInserter(result);

    while (count) {
        *inserter++ = generator(dis(gen));
        --count;
    }

    return result;
}

template<template<typename...> class Container,
         typename T,
         typename Generator = Internal::DefaultGenerator<T>>
auto generate_rnd(size_t count, T min, T max, Generator generator = {})
{
    return generate_rnd<Container<void>>(count, min, max, generator);
}


template<typename Container>
const auto& random_item(const Container& container)
{
    assert(!container.empty());
    return Internal::at(container, Internal::random(container.size() - 1));
}

template<template<typename...> class OverrideContainer = Internal::Empty,
         template<typename...> class Container,
         typename... CArgs>
auto random_items(const Container<CArgs...>& container, size_t count)
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container<CArgs...>, OverrideContainer<ElementType>>;

    if (!count)
        return ResultType();

    assert(container.size() > 0);

    ResultType result;
    Internal::tryReserve(result, count);
    auto inserter = Internal::getInserter(result);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, container.size() - 1);

    while (count) {
        *inserter++ = Internal::at(container, dis(gen));
        --count;
    }

    return result;
}


template<template<typename...> class OverrideContainer = Internal::Empty,
         template<typename...> class Container,
         typename... CArgs>
auto random_items_unique(const Container<CArgs...>& container, size_t count)
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container<CArgs...>, OverrideContainer<ElementType>>;

    if (!count)
        return ResultType();

    assert(static_cast<size_t>(container.size()) >= count);

    ResultType result;
    Internal::tryReserve(result, count);
    auto inserter = Internal::getInserter(result);

    std::random_device rd;
    std::mt19937 gen(rd());

    auto iterators = generate<std::vector>(static_cast<size_t>(container.size()), [it = std::cbegin(container)]() mutable { return it++; });
    std::shuffle(iterators.begin(), iterators.end(), gen);

    while (count) {
        *inserter++ = *iterators[--count];
    }

    return result;
}


template<typename WeightPredicate = std::nullptr_t,
         typename Container>
const auto& random_weighted_item(const Container& container, const WeightPredicate& weightPredicate = {})
{
    const auto pred = [&](){
        if constexpr (std::is_same_v<WeightPredicate, std::nullptr_t>)
            return [](const auto& x) { return x; };
        else
            return weightPredicate;
    }();

    assert(!container.empty());
    assert(std::all_of(container.begin(), container.end(), [&](const auto& x) { return pred(x) > 0; }));

    std::vector<uint64_t> cumulativeWeights;
    cumulativeWeights.reserve(static_cast<size_t>(container.size()));

    uint64_t sum {};
    for (const auto& x : container) {
        sum += static_cast<uint64_t>(pred(x));
        cumulativeWeights.push_back(sum);
    }

    const auto randomWeight = Internal::random<uint64_t>(sum - 1);
    const auto it = std::upper_bound(cumulativeWeights.begin(), cumulativeWeights.end(), randomWeight);
    const auto index = std::distance(cumulativeWeights.begin(), it);
    return Internal::at(container, index);
}

template<template<typename...> class OverrideContainer = Internal::Empty,
         typename WeightPredicate = std::nullptr_t,
         template<typename...> class Container,
         typename... CArgs>
auto random_weighted_items(const Container<CArgs...>& container,
                           size_t count,
                           const WeightPredicate& weightPredicate = {},
                           size_t smallContainerThreshold = 100)
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using ResultType = std::conditional_t<std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>, Container<CArgs...>, OverrideContainer<ElementType>>;

    const auto pred = [&](){
        if constexpr (std::is_same_v<WeightPredicate, std::nullptr_t>)
            return [](const auto& x) { return x; };
        else
            return weightPredicate;
    }();

    if (!count)
        return ResultType();

    assert(std::all_of(container.begin(), container.end(), [&](const auto& x) { return pred(x) > 0; }));

    std::random_device rd;
    std::mt19937 gen(rd());

    ResultType result;
    Internal::tryReserve(result, count);
    auto inserter = Internal::getInserter(result);

    if (container.size() < smallContainerThreshold) {
        uint64_t wSum {};
        for (const auto& x : container) {
            const auto w = static_cast<uint64_t>(pred(x));
            assert(wSum <= std::numeric_limits<uint64_t>::max() - w);
            wSum += w;
        }

        std::uniform_int_distribution<uint64_t> dis(0, wSum - 1);

        while (count) {
            const auto randomWeight = dis(gen);

            uint64_t sum {};
            for (const auto& x : container) {
                sum += static_cast<uint64_t>(pred(x));
                if (sum > randomWeight) {
                    *inserter++ = x;
                    break;
                }
            }

            --count;
        }

        return result;

    } else {
        std::vector<uint64_t> cumulativeWeights;
        cumulativeWeights.reserve(static_cast<size_t>(container.size()));

        uint64_t sum {};
        for (const auto& x : container) {
            const auto w = static_cast<uint64_t>(pred(x));
            assert(sum <= std::numeric_limits<uint64_t>::max() - w);
            sum += w;
            cumulativeWeights.push_back(sum);
        }

        std::uniform_int_distribution<uint64_t> dis(0, sum - 1);

        while (count) {
            const auto randomWeight = dis(gen);
            const auto it = std::upper_bound(cumulativeWeights.begin(), cumulativeWeights.end(), randomWeight);
            const auto index = std::distance(cumulativeWeights.begin(), it);
            *inserter++ = Internal::at(container, index);
            --count;
        }

        return result;
    }
}

template<template<typename...> class OverrideContainer = Internal::Empty,
         typename WeightPredicate = std::nullptr_t,
         template<typename...> class Container,
         typename... CArgs>
auto random_weighted_items_unique(const Container<CArgs...>& container,
                                  size_t count,
                                  const WeightPredicate& weightPredicate = std::nullptr_t())
{
    using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(*std::cbegin(container))>>;
    using ResultType = std::conditional_t<
        std::is_same_v<OverrideContainer<void>, Internal::Empty<void>>,
        Container<CArgs...>,
        OverrideContainer<ElementType>
        >;
    using IteratorDiff = typename Container<CArgs...>::difference_type;

    const auto pred = [&]() {
        if constexpr (std::is_same_v<WeightPredicate, std::nullptr_t>)
            return [](const auto& x) { return x; };
        else
            return weightPredicate;
    }();

    if (!count)
        return ResultType();

    assert(container.size() >= count);
    assert(std::all_of(container.begin(), container.end(), [&](const auto& x) { return pred(x) > 0; }));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    // Compute a key for each element using the Efraimidis–Spirakis algorithm:
    // key = u^(1/weight), where u is a random number in (0, 1].
    std::vector<std::pair<double, ElementType>> keys;
    keys.reserve(container.size());
    for (const auto& x : container) {
        const auto weight = static_cast<double>(pred(x));
        double u = dis(gen);
        if (u < std::numeric_limits<double>::min())
            u = std::numeric_limits<double>::min();
        double key = std::pow(u, 1.0 / weight);
        keys.emplace_back(key, x);
    }

    // Select elements with the highest keys.
    // Use std::nth_element for partial sorting to find the top 'count' elements.
    std::nth_element(keys.begin(), keys.begin() + static_cast<IteratorDiff>(count), keys.end(), std::greater<>());

    // Insert the selected elements into the result container.
    ResultType result;
    Internal::tryReserve(result, count);
    auto inserter = Internal::getInserter(result);
    for (size_t i = 0; i < count; ++i)
        *inserter++ = keys[i].second;

    // Sort. Actually not necessary, but it's nice to have the result sorted.
    std::sort(std::begin(result), std::end(result), std::greater<>());

    return result;
}

} // namespace utils_cpp
