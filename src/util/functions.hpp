
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

// To handle calling of events
class CMEvent {
  std::vector<void_func> func_pool; // to store added functions
public:
  inline void operator()(/*bool do_multithreading = false*/) {
    for (const auto& func : func_pool) func();
    /* TODO, FIX THREADING
    // Make enough threads for as many functions as we have.
    std::thread func_threads[func_pool.size()];
    // Start all threads with all our functions
    for (int i = 0; i < func_pool.size(); i++) {
      func_threads[i] = std::thread(func_pool[i]);
    }
    // Wait for the threads to finish
    for (auto& thread : func_threads)
      thread.join();*/
  }
  void add(void_func in) { func_pool.push_back(in); }
  void remove(void_func in) {
    for(int i = 0; i < func_pool.size(); i++) {
      if (func_pool[i] == in) {
        // Remove function from pool
        func_pool.erase(func_pool.begin() + i);
        // size has changed, need to recurse
        remove(in);
        return;
      }
    }
  }
};

// This is to handle before and after events happen
class CMEventGroup {
  CMEvent before_event;
  CMEvent during_event;
  CMEvent after_event;
public:
  inline void operator()() {
    before_event();
    during_event();
    after_event();
  }
  void REventBefore(void_func in) {before_event.add(in);};
  void REventDuring(void_func in) {during_event.add(in);};
  void REventAfter(void_func in) {after_event.add(in);};
};
