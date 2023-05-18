#pragma once
#include <optional>
#include <algorithm>
#include <type_traits>
#include <utility>

namespace utils_cpp {

template<typename T>
class SearchResult // Similar to std::optional<T>, but also has `index()` method
{
public:
    SearchResult() = default;
    SearchResult(const T& value, size_t index): m_result(std::make_pair(value, index)) {}

    SearchResult(const SearchResult<T>&) = default;
    SearchResult<T>& operator= (const SearchResult<T>&) = default;

    inline bool has_value() const { return m_result.has_value(); }
    inline explicit operator bool() const { return has_value(); };

    inline operator std::optional<T>() const { return has_value() ? std::optional<T>(value()) : std::optional<T>(); }

    inline const T& operator* () const { return value(); };
    inline T& operator* () { return value(); };
    inline const T* operator-> () const { return &value(); }
    inline T* operator-> () { return &value(); }
    inline const T& value() const { return m_result.value().first; }
    inline T& value() { return m_result.value().first; }

    T value_or(const T& altValue) const { return has_value() ? value() : altValue; }

    inline size_t index() const { return m_result.value().second; }

private:
    std::optional<std::pair<T, size_t>> m_result;
};

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
SearchResult<RT> find(const Container& container, const RT& value)
{
    auto it = std::find(std::cbegin(container), std::cend(container), value);
    return (it == std::cend(container)) ? SearchResult<RT>() : SearchResult<RT>(*it, std::distance(std::cbegin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>,
         typename Callable>
SearchResult<RT> find_if(const Container& container, const Callable& predicate)
{
    auto it = std::find_if(std::cbegin(container), std::cend(container), predicate);
    return (it == std::cend(container)) ? SearchResult<RT>() : SearchResult<RT>(*it, std::distance(std::cbegin(container), it));
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::value_type, T>>
std::optional<RT> find_in_set(const Container& container, const RT& value)
{
    auto it = container.find(value);
    return (it == std::cend(container)) ? std::optional<RT>() : std::optional<RT>(*it);
}

template<typename T = void,
         typename Container,
         typename RT = std::conditional_t<std::is_same_v<T, void>, typename Container::mapped_type, T>,
         typename KT = typename Container::key_type>
std::optional<RT> find_in_map(const Container& container, const KT& value)
{
    auto it = container.find(value);
    return (it == std::cend(container)) ? std::optional<RT>() : std::optional<RT>(it->second);
}

} // namespace utils_cpp
