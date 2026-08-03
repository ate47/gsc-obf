#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <stack>
#include <mutex>
#include <filesystem>
#include <cassert>
#include <functional>
#include <random>
#include <cstdint>
#include <cstdarg>
#include <span>

typedef uint8_t byte;

template<typename T, size_t N>
constexpr char (*ActsArraySizeCounter(T (&)[N]))[N];

#define ACTS_ARRAYSIZE(A) (sizeof(*ActsArraySizeCounter(A)))

#if __has_include(<xorstr.hpp>)
#include <xorstr.hpp>

#ifdef ACTS_SECURE_STRINGS
#define actssec(str) xorstr_(str)
#else
#define actssec(str) str
#endif
#else
#define actssec(str) str
#endif

#include <utils/logs.hpp>