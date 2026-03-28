<p align="center">
  <h1 align="center">utils-cpp</h1>
  <p align="center">
    <strong>Handy C++17 utilities and wrappers</strong>
  </p>
  <p align="center">
    Container helpers, safe integer casts, platform detection and more
  </p>
</p>

<p align="center">
  <a href="https://github.com/ihor-drachuk/utils-cpp/actions/workflows/ci.yml"><img src="https://github.com/ihor-drachuk/utils-cpp/actions/workflows/ci.yml/badge.svg?branch=master" alt="Build & Test"></a>
  <a href="https://github.com/ihor-drachuk/utils-cpp/blob/master/License.txt"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++ Standard">
  <img src="https://img.shields.io/badge/Platform-Windows%20|%20Linux%20|%20macOS-blueviolet.svg" alt="Platform">
</p>

---

A grab-bag of small C++17 utilities that fill gaps in the standard library — optional-like container search results, safe integer casts, lazy initialization, XOR helpers, VM detection and other bits accumulated over the years.

---

## Key Features

- **C++17** — some utilities overlap with C++20/23 features (ranges, `std::cmp_less`, ...) and can serve as lighter-weight alternatives for older compilers
- **Cross-platform** — tested on Linux (GCC, Clang), Windows (MSVC), and macOS
- **Qt-compatible** — optional support for Qt containers (QSet, QMap, etc.)
- **Tested** — comes with a Google Test suite

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
- **C++17** compatible compiler (MSVC, GCC, Clang)

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

### Container search

```cpp
#include <utils-cpp/container_utils.h>
#include <vector>

std::vector<int> data = {10, 20, 30, 40, 50};

if (auto result = utils_cpp::find_if(data, [](int x) { return x > 25; })) {
    std::cout << "Found: " << *result << " at index " << result.index() << "\n";
    // Found: 30 at index 2
}
```

### Filter + transform

```cpp
#include <utils-cpp/container_utils.h>
#include <vector>
#include <string>

std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

auto result = utils_cpp::copy_if_transform(
    numbers,
    [](int x) { return x % 2 == 0; },                    // predicate: keep evens
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
| `count`, `count_if` | Counting |
| `accumulate`, `reduce`, `transform_reduce` | Aggregation |
| `erase_if`, `erase_all`, `erase_all_vec`, `erase_one` | Element removal |
| `difference_sorted`, `difference_sorted_detailed` | Sorted container diff |
| `random_items`, `random_weighted_items`, `*_unique` | Random / weighted random selection |
| `generate`, `generate_rnd` | Container generation |

### Meta-programming & Type Traits

| Header | Description |
|--------|-------------|
| `function_traits.h` | Extract return type, argument types, arity from callables; `Functor` wrapper |
| `checkmethod.h` | Compile-time method detection (`CREATE_CHECK_METHOD`) |
| `comparison_traits.h` | Detect comparison operators (`is_comparable_eq`, etc.) |
| `tuple_utils.h` | Tuple manipulation, iteration, type checks |
| `variadic_tools.h` | Variadic template helpers (`variadic_and`, `variadic_or`) |

### Class Design Patterns

| Header | Description |
|--------|-------------|
| `default_ctor_ops.h` | `NO_COPY`, `NO_MOVE`, `NO_COPY_MOVE`, `DEFAULT_*`, `DEFAULT_OPS_EQ_NEQ` macros |
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
| `variant_tools.h` | Compare `std::variant` with plain types; `get_if` helper |

### Object Lifecycle

| Header | Description |
|--------|-------------|
| `lazy_init.h` | `lazy_init<T>`, `lazy_init_custom<T>` — deferred construction |
| `objects_pool.h` | Thread-safe object pooling |
| `sios.h` | Static Initialization Order Solution |

### Data Structures

| Header | Description |
|--------|-------------|
| `circularbuffer.h` | Fixed-size FIFO circular buffer |
| `middle_iterator.h` | Iterator traversing from center outward |
| `functor_iterator.h` | Wrap a callable as an input iterator |
| `value_or.h` | Chained optional access with fallbacks |

### System & Platform

| Header | Description |
|--------|-------------|
| `system_tools.h` | Device name, chassis type, admin rights detection |
| `vm_detector.h` | Virtual machine / hypervisor detection |
| `cpuid.h` | CPUID instruction wrapper (x86) |
| `stdin_listener.h` | Non-blocking stdin reader with echo/backspace support |
| `stdin_listener_native.h` | Line-based stdin listener |

### Miscellaneous

| Header | Description |
|--------|-------------|
| `threadid.h` | Unique thread identification |
| `string_hash.h` | Compile-time FNV-1a string hashing |
| `once.h` | Execute code once (`ONCE`, `ONCE_NAMED`) |
| `make_shared_ptr.h` | `make_shared_from`, `make_unique_from`, smart pointer wrappers |
| `safe_dynamic_cast.h` | Asserted dynamic casting |
| `warnings.h` | Cross-compiler warning management |
| `attributes.h` | Compiler-specific attributes |
| `safe_integers.h` | Safe integer casts and cross-type comparison |
| `xor.h` | Byte and buffer XOR operations |
| `algorithms.h` | Variadic `min`, `max`, `gcd`, `lcm` |
| `chrono_utils.h` | `ScopedTimer` for measuring elapsed time |
| `data_to_string.h` | Binary data to readable string |
| `gtest_printers.h` | Google Test pretty-printers for `std::chrono` types |
| `macros.h` | `STRINGIFY`, `TO_STRING`, conditional action/return macros |

---

## Building

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `UTILS_CPP_ENABLE_TESTS` | `OFF` | Build unit tests |
| `UTILS_CPP_ENABLE_BENCHMARK` | `OFF` | Build benchmarks |
| `UTILS_CPP_GTEST_SEARCH_MODE` | `Auto` | GTest detection: `Auto`, `Force`, `Skip` |
| `UTILS_CPP_TBB` | `Auto` | TBB linking (parallel execution policies): `Auto`, `ON`, `OFF` |

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
