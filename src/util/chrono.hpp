
/*
 *
 * Everything to do with time and time keeping
 *
 */

#include <chrono>

// My version of cathooks Timer class
class CatTimer {
private:
  // We use steady as it is constant and cant be changed
  std::chrono::time_point<std::chrono::steady_clock> last_time;
public:
  inline void Reset() {
    last_time = std::chrono::steady_clock::now();
  }
  inline bool CheckTime(std::chrono::steady_clock::duration dur) {
    return std::chrono::steady_clock::now() - last_time > dur;
  }
  inline bool ResetAfter(std::chrono::steady_clock::duration dur) {
    auto ret = CheckTime(dur);
    if (ret) Reset();
    return ret;
  }
};
