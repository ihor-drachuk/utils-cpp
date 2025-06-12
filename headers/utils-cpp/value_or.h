/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

#include <optional>
#include <type_traits>
#include <tuple>
#include <utility>

/*
            Description
  -------------------------------
  The "value_or" utility provides safe access to fields within std::optional objects,
  with support for both single field access and chained access through nested optional structures.

  This utility eliminates the need for verbose null-checking when accessing nested optional
  data structures, providing a clean and expressive API for handling potentially missing values.

  Key features:
  - Access to field within optional object with fallback value.
  - Chained access through multiple levels of nested optionals.

           Usage examples
  --------------------------------
  Single field access:
   /--
  |
  |  struct Person {
  |    std::string name;
  |    int age;
  |  };
  |
  |  std::optional<Person> optPerson;
  |
  |  auto name = utils_cpp::value_or(optPerson, &Person::name, "Noname");
  |
  |      -- Instead of --
  |
  |  auto name = optPerson.value_or(Person{"Noname", 0}).name;
  |
   \--

  Chained access:
   /--
  |
  |  struct DatabaseConfig {
  |    std::optional<std::string> host;
  |  };
  |
  |  struct Config {
  |    std::optional<DatabaseConfig> db;
  |  };
  |
  |  std::optional<Config> config;
  |
  |  auto host = utils_cpp::value_or(config, &Config::db, &DatabaseConfig::host, "localhost");
  |
   \--

  Fluent API:
   /--
  |  auto host = utils_cpp::chained_value_or(config)
  |                  .field(&Config::db)
  |                  .field(&DatabaseConfig::host)
  |                  .value_or("localhost");
   \--
 */

namespace utils_cpp {

namespace Internal {

// Helper to check if a type is std::optional
template<typename T>
struct is_optional : std::false_type {};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template<typename T>
constexpr bool is_optional_v = is_optional<T>::value;

// Implementation for single member access
template<typename T, typename FieldType, typename FallbackType>
constexpr auto value_or_impl(const std::optional<T>& opt, FieldType T::*field, FallbackType&& fallback)
    -> std::common_type_t<FieldType, std::decay_t<FallbackType>>
{
    if (opt.has_value()) {
        return opt.value().*field;
    }
    return std::forward<FallbackType>(fallback);
}

template<typename T, typename FieldType, typename FallbackType>
constexpr auto value_or_impl(std::optional<T>& opt, FieldType T::*field, FallbackType&& fallback)
    -> std::common_type_t<FieldType, std::decay_t<FallbackType>>
{
    if (opt.has_value()) {
        return opt.value().*field;
    }
    return std::forward<FallbackType>(fallback);
}

template<typename T, typename FieldType, typename FallbackType>
constexpr auto value_or_impl(std::optional<T>&& opt, FieldType T::*field, FallbackType&& fallback)
    -> std::common_type_t<FieldType, std::decay_t<FallbackType>>
{
    if (opt.has_value()) {
        return std::move(opt.value()).*field;
    }
    return std::forward<FallbackType>(fallback);
}

// Forward declaration for value_or_chained_impl
template<typename OptionalType, typename FallbackType, typename... Members>
constexpr auto value_or_chained_impl(OptionalType&& opt, FallbackType&& fallback, Members&&... members);

// Variadic chained implementation - base case (single member)
template<typename OptionalType, typename FallbackType, typename Member>
constexpr auto value_or_chained_impl(OptionalType&& opt, FallbackType&& fallback, Member&& member)
{
    return value_or_impl(std::forward<OptionalType>(opt), std::forward<Member>(member), std::forward<FallbackType>(fallback));
}

// Variadic chained implementation - recursive case
template<typename OptionalType, typename FallbackType, typename FirstMember, typename... RestMembers>
constexpr auto value_or_chained_impl(OptionalType&& opt, FallbackType&& fallback, FirstMember&& first, RestMembers&&... rest)
{
    if (opt.has_value()) {
        auto&& intermediate = opt.value().*first;

        if constexpr (is_optional_v<std::decay_t<decltype(intermediate)>>) {
            // If intermediate is already optional, pass it directly
            return value_or_chained_impl(
                std::forward<decltype(intermediate)>(intermediate),
                std::forward<FallbackType>(fallback),
                std::forward<RestMembers>(rest)...
                );
        } else {
            // Wrap non-optional intermediate in optional
            return value_or_chained_impl(
                std::optional<std::decay_t<decltype(intermediate)>>{std::forward<decltype(intermediate)>(intermediate)},
                std::forward<FallbackType>(fallback),
                std::forward<RestMembers>(rest)...
                );
        }
    } else {
        // When opt is empty, we need to deduce the final return type
        // This is done by recursively deducing through the member chain
        using T = typename std::decay_t<OptionalType>::value_type;
        using IntermediateType = decltype(std::declval<T>().*first);

        if constexpr (is_optional_v<std::decay_t<IntermediateType>>) {
            using NextType = typename std::decay_t<IntermediateType>::value_type;
            return value_or_chained_impl(
                std::optional<NextType>{},
                std::forward<FallbackType>(fallback),
                std::forward<RestMembers>(rest)...
                );
        } else {
            return value_or_chained_impl(
                std::optional<std::decay_t<IntermediateType>>{},
                std::forward<FallbackType>(fallback),
                std::forward<RestMembers>(rest)...
                );
        }
    }
}

// Helper to extract members and fallback from parameter pack
template<std::size_t... Is, typename OptionalType, typename... Args>
constexpr auto value_or_dispatch_impl(std::index_sequence<Is...>, OptionalType&& opt, Args&&... args)
{
    auto args_tuple = std::forward_as_tuple(std::forward<Args>(args)...);
    constexpr auto last_idx = sizeof...(Args) - 1;

    return value_or_chained_impl(
        std::forward<OptionalType>(opt),
        std::forward<typename std::tuple_element<last_idx, decltype(args_tuple)>::type>(std::get<last_idx>(args_tuple)),
        std::forward<typename std::tuple_element<Is, decltype(args_tuple)>::type>(std::get<Is>(args_tuple))...
        );
}

// Wrapper for fluent API
template<typename T>
class optional_chain
{
public:
    constexpr explicit optional_chain(std::optional<T> opt) : m_opt(std::move(opt)) {}

    // Chain field access
    template<typename FieldType>
    constexpr auto field(FieldType T::*field) &&
    {
        using NextType = std::decay_t<FieldType>;
        if (m_opt.has_value()) {
            auto&& value = m_opt.value().*field;
            if constexpr (is_optional_v<NextType>) {
                return optional_chain<typename NextType::value_type>{std::move(value)};
            } else {
                return optional_chain<NextType>{std::optional<NextType>{std::move(value)}};
            }
        } else {
            if constexpr (is_optional_v<NextType>) {
                return optional_chain<typename NextType::value_type>{std::nullopt};
            } else {
                return optional_chain<NextType>{std::nullopt};
            }
        }
    }

    // Get value or fallback
    template<typename FallbackType>
    constexpr auto value_or(FallbackType&& fallback) &&
    {
        return m_opt.value_or(std::forward<FallbackType>(fallback));
    }

    // Get the wrapped optional
    constexpr const std::optional<T>& get() const & { return m_opt; }
    constexpr std::optional<T> get() && { return std::move(m_opt); }

    // Implicit conversion to optional
    constexpr operator std::optional<T>() && { return std::move(m_opt); }

private:
    std::optional<T> m_opt;
};

} // namespace Internal


// Single field access
template<typename OptionalType, typename FieldType, typename T, typename FallbackType>
constexpr auto value_or(OptionalType&& opt, FieldType T::*field, FallbackType&& fallback)
{
    return Internal::value_or_impl(std::forward<OptionalType>(opt), field, std::forward<FallbackType>(fallback));
}

// Variadic chained field access - handles any number of members
template<typename OptionalType, typename... Args,
         typename = std::enable_if_t<(sizeof...(Args) >= 3)>> // At least 2 members + 1 fallback
constexpr auto value_or(OptionalType&& opt, Args&&... args)
{
    constexpr auto member_count = sizeof...(Args) - 1;
    return Internal::value_or_dispatch_impl(
        std::make_index_sequence<member_count>{},
        std::forward<OptionalType>(opt),
        std::forward<Args>(args)...
        );
}

// Fluent API factory function
template<typename T>
constexpr auto chained_value_or(std::optional<T> opt)
{
    return Internal::optional_chain<T>{std::move(opt)};
}

} // namespace utils_cpp
