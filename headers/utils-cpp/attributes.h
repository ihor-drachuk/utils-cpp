#pragma once
// Compiler Attributes

// UB-San realted code https://stackoverflow.com/a/34814667/1012586
// clang
#if defined(__has_feature)
  #if __has_feature(undefined_behavior_sanitizer)
    #define UTIL_CPP_ATTR_UB __attribute__((no_sanitize("undefined")))
  #endif
#endif
// gcc
#if !defined(UTIL_CPP_ATTR_UB) && defined(__SANITIZE_ADDRESS__)
    #define UTIL_CPP_ATTR_UB __attribute__((no_sanitize("undefined")))
#endif
// fallback
#ifndef UTIL_CPP_ATTR_UB
  #define UTIL_CPP_ATTR_UB
#endif