
/*
 *
 * Everything to do with time and time keeping
 *
 */
#pragma once

#include <chrono>

// My version of cathooks Timer class
class CatTimer {
private:
  // We use steady as it is constant and cant be changed
  std::chrono::time_point<std::chrono::steady_clock> last_time;
public:
  inline CatTimer& Reset() { // A reference in case of Init
    last_time = std::chrono::steady_clock::now();
    return *this;
  }
  inline bool CheckTime(std::chrono::steady_clock::duration dur) const {
    return std::chrono::steady_clock::now() - last_time > dur;
  }
  inline bool ResetAfter(std::chrono::steady_clock::duration dur) {
    auto ret = CheckTime(dur);
    if (ret) Reset();
    return ret;
  }
};
