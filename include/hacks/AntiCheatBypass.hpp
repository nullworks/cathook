#pragma once

#include <sdk.hpp>
#include <Settings.hpp>

namespace hacks::antianticheat
{
extern settings::Boolean enabled;

void SendNetMsg(INetMessage &msg);

bool CanSetCmdNum(int new_cmdnum);
} // namespace hacks::antianticheat
