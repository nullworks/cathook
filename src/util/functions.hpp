
/*
 *
 *  This file helps deal with our modular functions
 *  The purpose to keep modularity without
 *
 */

#pragma once

#include <thread>
#include <chrono>
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
  CMFunction(func_type func=[](args...){ return ret(); }) : func(func) {}
  inline auto operator()(args... a) { return func(a...); }
  inline void operator=(func_type _func) { func = _func; }
private:
  func_type func = nullptr;
};

// To handle calling of events
template <typename... args>
class CMEvent {
  using func_type = void(*)(args...);
protected:
  std::vector<func_type> func_pool; // to store added functions
public:
  inline void operator()(args... a /*bool do_multithreading = false*/) {
    for (const auto& func : func_pool) func(a...);
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
  void add(func_type in) { func_pool.push_back(in); }
  void remove(func_type in) {
    for(size_t i = 0; i < func_pool.size(); i++) {
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
public:
  CMEvent<> before_event;
  CMEvent<> during_event;
  CMEvent<> after_event;
  inline void operator()() {
    in_event = true;
    before_event();
    during_event();
    after_event();
    in_event = false;
  }
  inline void REventBefore(void_func in) {before_event.add(in);}
  inline void REventDuring(void_func in) {during_event.add(in);}
  inline void REventAfter(void_func in) {after_event.add(in);}
  inline bool GetInEvent(){return in_event;}
private:
  bool in_event;
};

// a class to make loop threading easier
// TODO: Still broken, pls fix
template<typename... args>
class ThreadedLoop {
public:
  ThreadedLoop(){}
  ThreadedLoop(void(*_thread_loop)(args...), args... a){
    Init(_thread_loop, a...);
  }
  void Init(void(*_thread_loop)(args...), args... a) {
    if (state != HALTED) return;
    thread_loop = _thread_loop;
    // This is the loop
    std::thread thread_construct([&](){
      state = RUNNING;
      while (state != STOP) // We loop while stop isnt true
        thread_loop(a...);
      state = HALTED;
    });
    thread_construct.detach();
  }
  ~ThreadedLoop(){
    state = STOP;
    // We hang the thread deconstructing this until the loop thread ends to prevent segfaulting
    while(state != HALTED)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
private:
  enum {RUNNING, STOP, HALTED};
  CMFunction<void(args...)> thread_loop;
  int state = HALTED;
};

// Is to be used like an ordered map, but by replacing hashing with iteration, it can be faster with a small amount of items
// It is very simple, it has limited uses but that shouldnt hurt much in the way i use it
template<typename T_Key, typename T_Value>
class CatFastMap {
public:
  using bucket_values = std::pair<T_Key, T_Value>;
  using bucket_type = std::vector<bucket_values>;
  CatFastMap(){}
  CatFastMap(const bucket_type& _bucket) {
    this = _bucket;
  }
  CatFastMap(std::initializer_list<bucket_values> _bucket) {
    bucket_type tmp = _bucket;
    *this = tmp;
  }
  // vector funcs
  inline void operator=(const bucket_type& input) {
    this->clear();
    for (const auto& i : input)
      this->insert(i); // We do this to multiple same keys
  }
  inline void clear() { this->bucket.clear(); }
  inline auto size() const { return this->bucket.size(); }
  inline auto begin() const {return this->bucket.begin();} // TODO: fix iterators
  inline auto end() const {return this->bucket.end();}
  // map funcs
  inline void insert(const bucket_values& in) {
    if (this->find(in.first) != this->end()) // Gotta retain normal map behaviour
      return;
    this->bucket.push_back(in);
  }
  inline void set(const bucket_values& in) { // use to set a variable inside the map, if it doesnt exist it constructs it
    this->erase(in.first);// TODO, fix below
    this->insert(in);
    /*auto find = this->find(in.first);
    if (find == this->end())
      this->insert(in);
    else { // TODO, fix
      find = in;*/
  }
  inline void erase(const T_Key& in){
    auto find = this->find(in);
    if (find != this->end())
      bucket.erase(find);
  }
  inline auto& operator[](const T_Key& i) { return this->find(i); }
  inline const auto& operator[](const T_Key& i) const { return this->find(i); }
  inline auto find(const T_Key& in) {
    for (int i = 0; i < this->bucket.size(); i++)
      if (this->bucket[i].first == in) return this->bucket.begin() + i;
    return this->bucket.end();
  }
  inline const auto find(const T_Key& in) const {
    for (int i = 0; i < this->bucket.size(); i++)
      if (this->bucket[i].first == in) return this->bucket.begin() + i;
    return this->bucket.end();
  }
private:
  bucket_type bucket;
};
