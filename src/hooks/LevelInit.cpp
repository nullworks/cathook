/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <hacks/hacklist.hpp>
#include "HookedMethods.hpp"
#include "MiscTemporary.hpp"
#if not LAGBOT_MODE
#include "hacks/Backtrack.hpp"
#endif

const char *skynum[] = { "sky_tf2_04",
                         "sky_upward",
                         "sky_dustbowl_01",
                         "sky_goldrush_01",
                         "sky_granary_01",
                         "sky_well_01",
                         "sky_gravel_01",
                         "sky_badlands_01",
                         "sky_hydro_01",
                         "sky_night_01",
                         "sky_nightfall_01",
                         "sky_trainyard_01",
                         "sky_stormfront_01",
                         "sky_morningsnow_01",
                         "sky_alpinestorm_01",
                         "sky_harvest_01",
                         "sky_harvest_night_01",
                         "sky_halloween",
                         "sky_halloween_night_01",
                         "sky_halloween_night2014_01",
                         "sky_island_01",
                         "sky_jungle_01",
                         "sky_invasion2fort_01",
                         "sky_well_02",
                         "sky_outpost_01",
                         "sky_coastal_01",
                         "sky_rainbow_01",
                         "sky_badlands_pyroland_01",
                         "sky_pyroland_01",
                         "sky_pyroland_02",
                         "sky_pyroland_03" };
static CatEnum skys({ "sky_tf2_04",
                      "sky_upward",
                      "sky_dustbowl_01",
                      "sky_goldrush_01",
                      "sky_granary_01",
                      "sky_well_01",
                      "sky_gravel_01",
                      "sky_badlands_01",
                      "sky_hydro_01",
                      "sky_night_01",
                      "sky_nightfall_01",
                      "sky_trainyard_01",
                      "sky_stormfront_01",
                      "sky_morningsnow_01",
                      "sky_alpinestorm_01",
                      "sky_harvest_01",
                      "sky_harvest_night_01",
                      "sky_halloween",
                      "sky_halloween_night_01",
                      "sky_halloween_night2014_01",
                      "sky_island_01",
                      "sky_jungle_01",
                      "sky_invasion2fort_01",
                      "sky_well_02",
                      "sky_outpost_01",
                      "sky_coastal_01",
                      "sky_rainbow_01",
                      "sky_badlands_pyroland_01",
                      "sky_pyroland_01",
                      "sky_pyroland_02",
                      "sky_pyroland_03" });
static CatVar
    skybox_changer(skys, "skybox_changer", "0", "Change Skybox to this skybox",
                   "Change Skybox to this skybox, only changes on map load");
static CatVar halloween_mode(CV_SWITCH, "halloween_mode", "0",
                             "Forced Halloween mode",
                             "forced tf_forced_holiday 2");

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(LevelInit, void, void *this_, const char *name)
{
#if not LAGBOT_MODE
    hacks::shared::backtrack::lastincomingsequencenumber = 0;
    hacks::shared::backtrack::sequences.clear();
#endif
    hacks::shared::autojoin::queuetime.update();
    firstcm = true;
#if not LAGBOT_MODE
    playerlist::Save();
#endif
#if not LAGBOT_MODE
    hacks::shared::lagexploit::bcalled = false;
#endif
#if ENABLE_VISUALS
    typedef bool (*LoadNamedSkys_Fn)(const char *);
    uintptr_t addr = gSignatures.GetEngineSignature(
        "55 89 E5 57 31 FF 56 8D B5 ? ? ? ? 53 81 EC 6C 01 00 00");
    static LoadNamedSkys_Fn LoadNamedSkys = LoadNamedSkys_Fn(addr);
    bool succ;
    logging::Info("Going to load the skybox");
    succ = LoadNamedSkys(skynum[(int) skybox_changer]);
    logging::Info("Loaded Skybox: %s", succ ? "true" : "false");
    ConVar *holiday = g_ICvar->FindVar("tf_forced_holiday");

    for (int i                       = 0; i < 32; i++)
        g_Settings.brute.brutenum[i] = 0;
    if (halloween_mode)
        holiday->SetValue(2);
    else if (holiday->m_nValue == 2)
        holiday->SetValue(0);
#endif

    g_IEngine->ClientCmd_Unrestricted("exec cat_matchexec");
#if not LAGBOT_MODE
    hacks::shared::aimbot::Reset();
    hacks::shared::backtrack::Init();
    chat_stack::Reset();
    hacks::shared::anticheat::ResetEverything();
    original::LevelInit(this_, name);
    hacks::shared::walkbot::OnLevelInit();
#endif
#if ENABLE_IPC
    if (ipc::peer)
    {
        ipc::peer->memory->peer_user_data[ipc::peer->client_id].ts_connected =
            time(nullptr);
    }
#endif
}
}
