
/*
 *
 * aa header
 *
 */

#include "../util/functional.hpp"

namespace features::antiaim {

// How many ticks to be "safe"
extern int AASafeTicks;

inline void AddAATicks(int i) {
  if (!AASafeTicks)
    AASafeTicks += i;
}

extern CMFunction<bool()> ShouldAA;

void Init();

}
