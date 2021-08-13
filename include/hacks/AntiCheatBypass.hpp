#pragma once

#include <sdk.hpp>
#include <Settings.hpp>

namespace hacks::tf2::antianticheat
{

extern settings::Boolean enabled;

void SendNetMsg(INetMessage &msg);

bool CanSetCmdNum(int new_cmdnum);
} // namespace hacks::tf2::antianticheat
