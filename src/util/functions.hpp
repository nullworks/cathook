
/*
 *
 *  This file helps deal with our modular functions
 *  The purpose to keep modularity without
 *
 */

#pragma once

#include <thread>
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
  using func_type = ret(*)(args...); // For std::function template like use
public:
  CMFunction(func_type _func) : func(_func) {}
  inline auto operator()(args... a) { return func(a...); }
  inline void operator=(func_type _func) { func = _func; }
private:
  func_type func = nullptr;
};

class CMFunctionGroup {
  std::vector<void_func> func_pool; // to store added functions
public:
  // TODO, threading
  inline void operator()(bool do_multithreading = false) {

    // If run requests not to do multithreading, we just run all the functions in order
    if (!do_multithreading) {
      for (auto func : func_pool) func();
      return;
    }

    // Note: It shouldnt matter how many cores the system has as the operating system can determine on its own(in theroy), which thread should be run at a time, but eventually everything should finish.
    // Make enough threads for as many functions as we have.
    std::thread func_threads[func_pool.size()];
    // Start all threads with all our functions
    for (int i = 0; i < func_pool.size(); i++) {
      func_threads[i] = std::thread(func_pool[i]);
    }
    // Wait for the threads to finish
    for (auto& thread : func_threads)
      thread.join();
  }

  void operator+(void_func in) { func_pool.push_back(in); }
};
