#pragma once
#include "config.h"
#if ENABLE_NULLNEXUS
#include <string>

namespace nullnexus
{
bool sendmsg(std::string &msg);
} // namespace nullnexus
#endif
