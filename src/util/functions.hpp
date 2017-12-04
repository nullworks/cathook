
/*
 *
 *  This file helps deal with our modular functions
 *  The purpose to keep modularity without
 *
 */

#pragma once

#include <vector>

// Useful
using void_func = void(*)(void);

// Use when you have a function that you wish to be modular
// The point of this is to be faster than std::function but retain its usability
// Credits to fission, thanks bby <3
template<typename>
class CMFunction;

template <typename ret, typename... args>
class CMFunction <ret(args...)> {
  using func_type = ret(*)(args...);
public:
  CMFunction(func_type _func) : func(_func) {}
  inline auto operator()(args... a) { return func(a...); }
  inline void operator=(func_type _func) { func = _func; }
private:
  func_type func = nullptr;
};

class CMFunctionGroup {
private:
  std::vector<void_func> func_pool;
public:
  inline void operator()() { for (const auto& func : func_pool) func(); }
  void operator+(void_func in) { func_pool.push_back(in); }
};
