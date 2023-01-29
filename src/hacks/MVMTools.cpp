#include "common.hpp"

namespace hacks::tf2::mvmtools
{
static settings::Boolean auto_revive("mvmtools.instant-revive", "false");
static settings::Boolean auto_f4("mvmtools.auto-f4", "false");

static Timer revive_timer{};
static Timer ready_timer{};

static void CreateMove()
{
    if (CE_INVALID(LOCAL_E))
        return;
    // only in MVM
    if (!g_pGameRules->isPVEMode)
        return;

    if (auto_revive && !LOCAL_E->m_bAlivePlayer() && revive_timer.test_and_set(1000))
    {
        auto *kv = new KeyValues("MVM_Revive_Response");
        kv->SetInt("accepted", 1);
        g_IEngine->ServerCmdKeyValues(kv);
    }
    if (auto_f4 && g_pGameRules->roundmode == 10 && ready_timer.test_and_set(10000))
        g_IEngine->ServerCmd("tournament_player_readystate 1");
}

static InitRoutine init([]() { EC::Register(EC::CreateMove, CreateMove, "mvmtools_createmove"); });
} // namespace hacks::tf2::mvmtools
