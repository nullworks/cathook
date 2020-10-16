#pragma once
#include "settings/Bool.hpp"
class INetMessage;
namespace hacks::tf2::warp
{
void SendNetMessage(INetMessage &msg);
void CL_SendMove_hook();
} // namespace hacks::tf2::warp
