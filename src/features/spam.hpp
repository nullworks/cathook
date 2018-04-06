
/*
 *
 *
 *
 *
 */

#include "../util/functional.hpp"

namespace features::spam {

// Please set in your module to enable chat spam features
extern CMFunction<void(const char*)> SayChat;

void Init();

}
