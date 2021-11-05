/*
 * Created on 3rd November 2019
 * Author: LightCat
 */
#include "reclasses.hpp"
#include "HookedMethods.hpp"

static settings::Boolean enabled{ "misc.backpack-expander.enabled", "false" };
static settings::Int slot_count{ "misc.backpack-expander.slot-count", "3000" };
namespace hooked_methods
{
// Note that this is of the type CTFPlayerInventory *
DEFINE_HOOKED_METHOD(GetMaxItemCount, int, int *)
{
    // Max backpack slots ez gg
    return *slot_count;
}
} // namespace hooked_methods

static Timer hook_cooldown{};
namespace hacks::backpack_expander
{

void Paint()
{
    if (!enabled)
        return;
    if (hook_cooldown.test_and_set(1000))
    {
        static auto invmng = re::CTFInventoryManager::GTFInventoryManager();
        if (invmng)
        {
            auto inv = invmng->GTFPlayerInventory();
            if (!hooks::inventory.IsHooked(inv))
            {
                hooks::inventory.Set(inv, 0);
                hooks::inventory.HookMethod(HOOK_ARGS(GetMaxItemCount));
                hooks::inventory.Apply();
            }
        }
    }
}

static InitRoutine init(
    []()
    {
#if !ENFORCE_STREAM_SAFETY
        EC::Register(EC::Paint, Paint, "bpexpander_paint");
        EC::Register(
            EC::Shutdown, []() { hooks::inventory.Release(); }, "backpack_expander_shutdown");
        enabled.installChangeCallback(
            [](settings::VariableBase<bool> &, bool after)
            {
                if (!after)
                    hooks::inventory.Release();
            });
#endif
    });
} // namespace hacks::backpack_expander
