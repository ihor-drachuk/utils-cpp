#pragma once
#include <optional>
#include <algorithm>
#include <type_traits>

namespace UtilsCpp {

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
std::optional<RT> find(const Container& container, const RT& value)
{
    auto it = std::find(container.cbegin(), container.cend(), value);
    return (it == container.cend()) ? std::optional<RT>() : std::optional<RT>(*it);
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable>
std::optional<RT> find_if(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(container.cbegin(), container.cend(), predicate);
    return (it == container.cend()) ? std::optional<RT>() : std::optional<RT>(*it);
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
std::optional<RT> find_in_set(const Container& container, const RT& value)
{
    auto it = container.find(value);
    return (it == container.cend()) ? std::optional<RT>() : std::optional<RT>(*it);
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::mapped_type, T>,
         typename KT = typename Container::key_type>
std::optional<RT> find_in_map(const Container& container, const KT& value)
{
    auto it = container.find(value);
    return (it == container.cend()) ? std::optional<RT>() : std::optional<RT>(it->second);
}

} // namespace UtilsCpp
