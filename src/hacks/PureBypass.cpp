#include <settings/Bool.hpp>
#include "common.hpp"

namespace hacks::purebypass
{
hooks::VMTHook svpurehook{};
static void (*orig_RegisterFileWhilelist)(void *, void *, void *);
// epic hook
static void RegisterFileWhitelist(void *, void *, void *)
{
}

static InitRoutine init(
    []
    {
        svpurehook.Set(g_IFileSystem);
        svpurehook.HookMethod(RegisterFileWhitelist, offsets::RegisterFileWhitelist(), &orig_RegisterFileWhilelist);
        svpurehook.Apply();
    });
} // namespace hacks::purebypass
