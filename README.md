<p align="center">
  <h1 align="center">utils-cpp</h1>
  <p align="center">
    <strong>Modern C++ Utility Library</strong>
  </p>
  <p align="center">
    Type-safe utilities for everyday C++ development
  </p>
</p>

<p align="center">
  <a href="https://github.com/ihor-drachuk/utils-cpp/actions/workflows/ci.yml"><img src="https://github.com/ihor-drachuk/utils-cpp/actions/workflows/ci.yml/badge.svg?branch=master" alt="Build & Test"></a>
  <a href="https://github.com/ihor-drachuk/utils-cpp/blob/master/License.txt"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++ Standard">
  <img src="https://img.shields.io/badge/Platform-Windows%20|%20Linux%20|%20macOS-blueviolet.svg" alt="Platform">
</p>

---

## Key Features

- **Modern C++17** — Leverages latest language features and best practices
- **Cross-platform** — Tested on Linux (GCC, Clang), Windows (MSVC), and macOS
- **Type-safe** — Compile-time checks with templates and SFINAE
- **Qt-compatible** — Special support for Qt containers
- **Well-tested** — Comprehensive test suite with Google Test

---

## Table of Contents

- [Requirements](#requirements)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [What's Included](#whats-included)
- [Building](#building)
- [License](#license)

---

## Requirements

- **CMake** 3.16+
- **C++17** compatible compiler:
  - GCC 7+
  - Clang 5+
  - MSVC 2017+

---

## Installation

### Option #1: Auto-download

Requirements: CMake 3.16+, git

```cmake
include(FetchContent)
FetchContent_Declare(utils-cpp
    GIT_REPOSITORY https://github.com/ihor-drachuk/utils-cpp.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(utils-cpp)

target_link_libraries(YourProject PRIVATE utils-cpp)  # Replace "YourProject"
```

### Option #2: As submodule

```bash
git submodule add https://github.com/ihor-drachuk/utils-cpp.git third-party/utils-cpp
```

```cmake
add_subdirectory(third-party/utils-cpp)
target_link_libraries(YourProject PRIVATE utils-cpp)
```

---

## Quick Start

### Example 1: Elegant Container Search

Forget `iterator == container.end()` checks. Use expressive search results:

```cpp
#include <utils-cpp/container_utils.h>
#include <vector>

std::vector<int> data = {10, 20, 30, 40, 50};

// Clean, readable code
if (auto result = UtilsCpp::find_if(data, [](int x) { return x > 25; })) {
    std::cout << "Found: " << *result << " at index " << result.index() << std::endl;
    // Output: Found: 30 at index 2
}

// Works with values too
if (auto found = UtilsCpp::find(data, 40)) {
    std::cout << "Value exists at index " << found.index() << std::endl;
}
```

### Example 2: Filter and Transform in One Pass

```cpp
#include <utils-cpp/container_utils.h>
#include <vector>
#include <string>

std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Filter even numbers and convert to strings - single pass
auto result = UtilsCpp::copy_if_transform(
    numbers,
    [](int x) { return x % 2 == 0; },              // predicate: keep evens
    [](int x) { return "Value: " + std::to_string(x); }  // transform to string
);

// result: {"Value: 2", "Value: 4", "Value: 6", "Value: 8", "Value: 10"}
```

---

## What's Included

### Container Utilities
`<utils-cpp/container_utils.h>`

| Function | Description |
|----------|-------------|
| `find`, `find_if`, `find_cref`, `find_ref` | Search with optional index tracking |
| `find_in_map`, `find_in_map_cref` | Map/dictionary lookups |
| `contains`, `contains_if`, `contains_set`, `contains_map` | Existence checks |
| `index_of`, `index_of_if` | Index lookup |
| `all_of`, `any_of`, `none_of` | Quantifier checks (value and predicate variants) |
| `copy_if`, `transform`, `copy_if_transform` | Filtering and transformation |

### Meta-programming & Type Traits

| Header | Description |
|--------|-------------|
| `function_traits.h` | Extract return type, argument types, arity from callables |
| `checkmethod.h` | Compile-time method detection (`CREATE_CHECK_METHOD`) |
| `comparison_traits.h` | Detect comparison operators (`is_comparable_eq`, etc.) |
| `tuple_utils.h` | Tuple manipulation, iteration, type checks |
| `variadic_tools.h` | Variadic template helpers (`variadic_and`, `variadic_or`) |

### Class Design Patterns

| Header | Description |
|--------|-------------|
| `copy_move.h` | `NO_COPY`, `NO_MOVE`, `NO_COPY_MOVE`, `DEFAULT_*` macros |
| `declareptr.h` | Smart pointer typedefs (`DECLARE_PTR`) |
| `pimpl.h` | PIMPL idiom helper (`DECLARE_PIMPL`) |
| `singleton.h` | `Singleton<T>`, `SingletonGlobal<T>` |
| `scoped_guard.h` | RAII cleanup (`CreateScopedGuard`) |
| `overloaded.h` | Visitor pattern for `std::variant` |

### Struct Comparisons

| Header | Description |
|--------|-------------|
| `struct_ops.h` | Generate `==`, `!=`, `<`, `>`, `<=`, `>=` via `tie()` |
| `struct_ops2.h` | Float-aware comparisons with epsilon |
| `struct_ops3.h` | Multi-inheritance comparison support |
| `variant_eq_comparison.h` | Compare `std::variant` with plain types |

### Object Lifecycle

| Header | Description |
|--------|-------------|
| `lazyinit.h` | `LazyInit<T>` — deferred object creation |
| `objects_pool.h` | Thread-safe object pooling |
| `sios.h` | Static Initialization Order Solution |

### Data Structures

| Header | Description |
|--------|-------------|
| `circularbuffer.h` | Fixed-size FIFO circular buffer |
| `middle_iterator.h` | Iterator traversing from center outward |

### Miscellaneous

| Header | Description |
|--------|-------------|
| `threadid.h` | Unique thread identification |
| `string_hash.h` | Compile-time FNV-1a string hashing |
| `once.h` | Execute code once (`ONCE`, `ONCE_NAMED`) |
| `as_const.h` | Convert to const reference |
| `make_shared_from.h` | Create smart pointers from existing objects |
| `safe_dynamic_cast.h` | Asserted dynamic casting |
| `warnings.h` | Cross-compiler warning management |
| `attributes.h` | Compiler-specific attributes |
| `macros.h` | `STRINGIFY`, `TO_STRING` |

---

## Building

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `UTILS_CPP_ENABLE_TESTS` | `OFF` | Build unit tests |
| `UTILS_CPP_ENABLE_BENCHMARK` | `OFF` | Build benchmarks |
| `UTILS_CPP_GTEST_SEARCH_MODE` | `Auto` | GTest detection: `Auto`, `Force`, `Skip` |

### Commands

```bash
# Configure with tests
cmake -B build -DUTILS_CPP_ENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release -j$(nproc)

# Run tests
ctest --test-dir build --output-on-failure
```

---

## License

MIT License — see [License.txt](License.txt) for details.

Copyright (c) 2018-2026 Ihor Drachuk

---

## Author

**Ihor Drachuk** — [ihor-drachuk-libs@pm.me](mailto:ihor-drachuk-libs@pm.me)

[GitHub](https://github.com/ihor-drachuk/utils-cpp)
