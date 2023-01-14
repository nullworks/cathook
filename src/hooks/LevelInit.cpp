/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <hacks/hacklist.hpp>
#include <settings/Bool.hpp>
#if ENABLE_VISUALS
#include <menu/GuiInterface.hpp>
#endif
#include "HookedMethods.hpp"
#include "MiscTemporary.hpp"
#include "navparser.hpp"
#include "AntiAntiAim.hpp"

static settings::Boolean halloween_mode{ "misc.force-halloween", "false" };
static settings::Int skybox_changer{ "misc.skybox-override", "0" };
extern settings::Boolean random_name;
extern settings::String force_name;
extern std::string name_forced;

const char *skynum[] = { "", "sky_tf2_04", "sky_upward", "sky_dustbowl_01", "sky_goldrush_01", "sky_granary_01", "sky_well_01", "sky_gravel_01", "sky_badlands_01", "sky_hydro_01", "sky_night_01", "sky_nightfall_01", "sky_trainyard_01", "sky_stormfront_01", "sky_morningsnow_01", "sky_alpinestorm_01", "sky_harvest_01", "sky_harvest_night_01", "sky_halloween", "sky_halloween_night_01", "sky_halloween_night2014_01", "sky_island_01", "sky_jungle_01", "sky_invasion2fort_01", "sky_well_02", "sky_outpost_01", "sky_coastal_01", "sky_rainbow_01", "sky_badlands_pyroland_01", "sky_pyroland_01", "sky_pyroland_02", "sky_pyroland_03" };

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(LevelInit, void, void *this_, const char *name)
{
    firstcm = true;
    // nav::init = false;
    playerlist::Save();
#if ENABLE_VISUALS
#if ENABLE_GUI
    gui::onLevelLoad();
#endif
    if (skybox_changer)
    {
        typedef bool (*LoadNamedSkys_Fn)(const char *);
        uintptr_t addr                        = gSignatures.GetEngineSignature("55 89 E5 57 31 FF 56 8D B5 ? ? ? ? 53 81 EC 6C 01 00 00");
        static LoadNamedSkys_Fn LoadNamedSkys = LoadNamedSkys_Fn(addr);
        bool succ;
        logging::Info("Going to load the skybox");
        succ = LoadNamedSkys(skynum[(int) skybox_changer]);
        logging::Info("Loaded Skybox: %s", succ ? "true" : "false");
    }
    ConVar *holiday = g_ICvar->FindVar("tf_forced_holiday");
    if (halloween_mode)
        holiday->SetValue(2);
    else if (holiday->m_nValue == 2)
        holiday->SetValue(0);
#endif
    hacks::shared::anti_anti_aim::resolver_map.clear();
    g_IEngine->ClientCmd_Unrestricted("exec cat_matchexec");
    entity_cache::array.reserve(500);
    chat_stack::Reset();
    original::LevelInit(this_, name);
    EC::run(EC::LevelInit);
#if ENABLE_IPC
    if (ipc::peer)
    {
        ipc::peer->memory->peer_user_data[ipc::peer->client_id].ts_connected = time(nullptr);
    }
#endif
    if (*random_name)
    {
        static TextFile file;
        if (file.TryLoad("names.txt"))
            name_forced = file.lines.at(rand() % file.lines.size());
    }
    else
        name_forced = "";
}
} // namespace hooked_methods
