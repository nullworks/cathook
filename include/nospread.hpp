#pragma once

struct INetMessage;
struct bf_read;

namespace hacks::nospread
{
extern bool is_syncing;
bool SendNetMessage(INetMessage *);
void SendNetMessagePost();
bool DispatchUserMessage(bf_read *, int);
void CL_SendMove_hook();
} // namespace hacks::nospread
