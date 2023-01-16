/*
 * CatBot.cpp
 *
 *  Created on: Dec 30, 2017
 *      Author: nullifiedcat
 */

#include <settings/Bool.hpp>
#include "CatBot.hpp"
#include "common.hpp"
#include "hack.hpp"
#include "PlayerTools.hpp"
#include "e8call.hpp"
#include "NavBot.hpp"
#include "navparser.hpp"
#include "SettingCommands.hpp"
#include "glob.h"

namespace hacks::shared::catbot
{
static settings::Boolean auto_disguise{ "misc.autodisguise", "true" };

static settings::Int abandon_if_ipc_bots_gte{ "cat-bot.abandon-if.ipc-bots-gte", "0" };
static settings::Int abandon_if_humans_lte{ "cat-bot.abandon-if.humans-lte", "0" };
static settings::Int abandon_if_players_lte{ "cat-bot.abandon-if.players-lte", "0" };

static settings::Boolean micspam{ "cat-bot.micspam.enable", "false" };
static settings::Int micspam_on{ "cat-bot.micspam.interval-on", "3" };
static settings::Int micspam_off{ "cat-bot.micspam.interval-off", "60" };

static settings::Boolean auto_crouch{ "cat-bot.auto-crouch", "false" };
static settings::Boolean always_crouch{ "cat-bot.always-crouch", "false" };
static settings::Boolean random_votekicks{ "cat-bot.votekicks", "false" };
static settings::Boolean votekick_rage_only{ "cat-bot.votekicks.rage-only", "false" };
static settings::Boolean autoReport{ "cat-bot.autoreport", "true" };
static settings::Boolean autovote_map{ "cat-bot.autovote-map", "true" };

static settings::Boolean mvm_autoupgrade{ "mvm.autoupgrade", "false" };

settings::Boolean catbotmode{ "cat-bot.enable", "false" };
settings::Boolean anti_motd{ "cat-bot.anti-motd", "false" };

// These are used for randomly loading a config on respawn for the bots

// Master switch
static settings::Boolean enable_reload{ "cat-bot.autoload.enable", "false" };

// Misc Settings
static settings::Float reload_chance{ "cat-bot.autoload.chance", "100" };
static settings::Int reload_deaths{ "cat-bot.autoload.deaths", "0" };
static settings::Boolean load_same_config{ "cat-bot.autoload.load-same-config", "true" };

// Config to load
static settings::String conf1{ "cat-bot.autoload.conf1", "bot_*" };
static settings::String conf2{ "cat-bot.autoload.conf2", "" };
static settings::String conf3{ "cat-bot.autoload.conf3", "" };

// Should that config get loaded?
static settings::Boolean conf1_enable{ "cat-bot.autoload.conf1.enable", "false" };
static settings::Boolean conf2_enable{ "cat-bot.autoload.conf2.enable", "false" };
static settings::Boolean conf3_enable{ "cat-bot.autoload.conf3.enable", "false" };

struct catbot_user_state
{
    int treacherous_kills{ 0 };
};

static boost::unordered_flat_map<unsigned, catbot_user_state> human_detecting_map{};

int globerr(const char *path, int eerrno)
{
    logging::Info("%s: %s\n", path, strerror(eerrno));
    // let glob() keep going
    return 0;
}

bool hasEnding(std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length())
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    else
        return false;
}

std::vector<std::string> config_list(std::string in)
{
    std::string complete_in = paths::getConfigPath() + "/" + in;
    if (!hasEnding(complete_in, ".conf"))
        complete_in = complete_in + ".conf";
    std::vector<std::string> config_vec;
    size_t i;
    int flags = 0;
    glob_t results;
    int ret;

    flags |= 0;
    ret = glob(complete_in.c_str(), flags, globerr, &results);
    if (ret != 0)
    {
        std::string ret_str;
        switch (ret)
        {
        case GLOB_ABORTED:
            ret_str = "filesystem problem";
            break;
        case GLOB_NOMATCH:
            ret_str = "no match of pattern";
            break;
        case GLOB_NOSPACE:
            ret_str = "out of memory";
            break;
        default:
            ret_str = "Unknown problem";
            break;
        }

        logging::Info("problem with %s (%s), stopping early\n", in.c_str(), ret_str.c_str());
        return config_vec;
    }

    for (i = 0; i < results.gl_pathc; ++i)
        // /configs/ is 9 extra chars i have to remove
        config_vec.push_back(std::string(results.gl_pathv[i]).substr(paths::getDataPath().length() + 9));

    globfree(&results);
    return config_vec;
}
static std::string blacklist;
static int deaths = 0;
void on_killed_by(int userid)
{
    if (enable_reload)
    {
        // Should we load yet?
        bool should_load = false;

        // Default to chance if no deaths are set
        if (!reload_deaths)
        {
            // RNG
            if (UniformRandomInt(0, 99) < *reload_chance)
                should_load = true;
        }
        // You died more than the specified amount of times
        else if (deaths++ >= *reload_deaths)
        {
            should_load = true;
            deaths      = 0;
        }
        if (should_load)
        {
            // Candidates for loading
            std::vector<std::string> temp_candidates;
            std::vector<std::string> load_candidates;
            if (conf1_enable)
            {
                temp_candidates = config_list(*conf1);
                for (auto &i : temp_candidates)
                    load_candidates.push_back(i);
            }
            if (conf2_enable)
            {
                temp_candidates = config_list(*conf2);
                for (auto &i : temp_candidates)
                    load_candidates.push_back(i);
            }
            if (conf3_enable)
            {
                temp_candidates = config_list(*conf3);
                for (auto &i : temp_candidates)
                    load_candidates.push_back(i);
            }
            // Remove blacklisted
            if (!load_same_config)
                for (auto it = load_candidates.begin(); it != load_candidates.end();)
                {
                    if (*it == blacklist)
                        load_candidates.erase(it);
                    else
                        it++;
                }
            if (!load_candidates.empty())
            {
                // Load the config
                std::string to_load  = load_candidates.at(UniformRandomInt(0, load_candidates.size() - 1));
                to_load              = to_load.substr(0, to_load.size() - 5);
                std::string load_cmd = "cat_load " + to_load;
                g_IEngine->ClientCmd_Unrestricted(load_cmd.c_str());
                if (!load_same_config)
                    blacklist = to_load;
            }
        }
    }
}

void do_random_votekick()
{
    std::vector<int> targets;
    player_info_s local_info;

    if (CE_BAD(LOCAL_E) || !GetPlayerInfo(LOCAL_E->m_IDX, &local_info))
        return;
    for (int i = 1; i < g_GlobalVars->maxClients; ++i)
    {
        player_info_s info;
        if (!GetPlayerInfo(i, &info) || !info.friendsID)
            continue;
        if (g_pPlayerResource->GetTeam(i) != g_pLocalPlayer->team)
            continue;
        if (info.friendsID == local_info.friendsID)
            continue;
        auto &pl = playerlist::AccessData(info.friendsID);
        if (votekick_rage_only && pl.state != playerlist::k_EState::RAGE)
            continue;
        if (pl.state != playerlist::k_EState::RAGE && pl.state != playerlist::k_EState::DEFAULT)
            continue;

        targets.push_back(info.userID);
    }

    if (targets.empty())
        return;

    int target = targets[rand() % targets.size()];
    player_info_s info;
    if (!GetPlayerInfo(GetPlayerForUserID(target), &info))
        return;
    hack::ExecuteCommand("callvote kick \"" + std::to_string(target) + " cheating\"");
}

// Get Muh money
int GetMvmCredits()
{
    if (CE_GOOD(LOCAL_E))
        return NET_INT(RAW_ENT(LOCAL_E), 0x2f50);
    return 0;
}

static CatCommand debug_money("debug_mvmmoney", "Print MVM Money", []() { logging::Info("%d", GetMvmCredits()); });
// Store information
struct Posinfo
{
    float x;
    float y;
    float z;
    std::string lvlname;
    Posinfo(float _x, float _y, float _z, std::string _lvlname)
    {
        x       = _x;
        y       = _y;
        z       = _z;
        lvlname = _lvlname;
    }
    Posinfo(){};
};
struct Upgradeinfo
{
    int id;
    int cost;
    int clazz;
    // Higher = better
    int priority;
    int priority_falloff;
    Upgradeinfo(){};
    Upgradeinfo(int _id, int _cost, int _clazz, int _priority, int _priority_falloff)
    {
        id               = _id;
        cost             = _cost;
        clazz            = _clazz;
        priority         = _priority;
        priority_falloff = _priority_falloff;
    }
};
static std::vector<Upgradeinfo> upgrade_list;

static bool inited_upgrades = false;
// Pick a upgrade
Upgradeinfo PickUpgrade()
{
    if (!inited_upgrades)
    {
        // Damage ( Important )
        upgrade_list.push_back({ 0, 400, tf_sniper, 4, 1 });
        // Projectile Penetration ( Good )
        upgrade_list.push_back({ 12, 400, tf_sniper, 5, 500 });
        // Explosive Headshot
        upgrade_list.push_back({ 40, 350, tf_sniper, 6, 2 });
        // +50% Ammo ( Basically least valuable upgrade for primary )
        upgrade_list.push_back({ 6, 250, tf_sniper, 3, 1 });
        // +20% Reload Speed ( Pretty important )
        upgrade_list.push_back({ 35, 250, tf_sniper, 6, 1 });
        // +25 Health on kill ( It's meh yet you need it to not die )
        upgrade_list.push_back({ 11, 200, tf_sniper, 3, 2 });
        // +25% Faster charge ( Good for "Wait for charge" catbots )
        upgrade_list.push_back({ 17, 200, tf_sniper, 4, 1 });
        inited_upgrades = true;
    }
    int highest_priority = INT_MIN;
    std::vector<Upgradeinfo *> potential_upgrades;
    for (auto &i : upgrade_list)
    {
        // Don't want wrong class
        if (i.clazz != g_pLocalPlayer->clazz)
            continue;
        // Can't buy something we can't afford lol
        if (i.cost > GetMvmCredits())
            continue;
        // Not a Priority right now
        if (i.priority < highest_priority)
            continue;
        // Clear out everything incase a higher priority is found
        if (i.priority > highest_priority)
            potential_upgrades.clear();
        highest_priority = i.priority;
        potential_upgrades.push_back(&i);
    }
    int vec_size = potential_upgrades.size();
    if (!vec_size)
        return { -1, -1, -1, -1, -1 };
    else
    {
        auto choosen_element = potential_upgrades[rand() % vec_size];
        // Less important after an upgrade
        choosen_element->priority -= choosen_element->priority_falloff;
        return *choosen_element;
    }
}
static std::vector<Posinfo> spot_list;
// Upgrade Navigation
/*void NavUpgrade()
{
    std::string lvlname = g_IEngine->GetLevelName();
    std::vector<Posinfo> potential_spots{};

    for (auto &i : spot_list)
    {
        if (lvlname.find(i.lvlname) != lvlname.npos)
            potential_spots.push_back({ i.x, i.y, i.z, lvlname });
    }
    Posinfo best_spot{};
    float best_score = FLT_MAX;
    for (auto &i : potential_spots)
    {
        Vector pos  = { i.x, i.y, 0.0f };
        float score = pos.DistTo(LOCAL_E->m_vecOrigin());
        if (score < best_score)
        {
            best_spot  = i;
            best_score = score;
        }
    }
    Posinfo to_path                        = best_spot;
    hacks::tf2::NavBot::task::current_task = hacks::tf2::NavBot::task::outofbounds;
    bool success                           = nav::navTo(Vector{ to_path.x, to_path.y, to_path.z }, 8, true, true);
    if (!success)
    {
        logging::Info("No valid spots found!");
        hacks::tf2::NavBot::task::current_task = hacks::tf2::NavBot::task::none;
        return;
    }
}

static bool run = false;
static Timer run_delay;
static Timer buy_upgrade;
static InitRoutine init_routine([]() {
    EC::Register(
        EC::Paint,
        []() {
            if (run && run_delay.test_and_set(200))
            {
                run                = false;
                auto upgrade_panel = g_CHUD->FindElement("CHudUpgradePanel");
                typedef int (*CancelUpgrade_t)(CHudElement *);
                static uintptr_t addr = (unsigned) e8call((void *) (gSignatures.GetClientSignature("E8 ? ? ? ? 8B 3D ? ? ? ? 8D 4B 28 ") + 1));
                if (upgrade_panel)
                {
                    static CancelUpgrade_t CancelUpgrade_fn = CancelUpgrade_t(addr);
                    CancelUpgrade_fn(upgrade_panel);
                }
            }
        },
        "mvmupgrade_paint");
    EC::Register(
        EC::CreateMove,
        []() {
            if (!*mvm_autoupgrade)
                return;
            std::string lvlname = g_IEngine->GetLevelName();
            if (lvlname.find("mvm_") == lvlname.npos)
                return;
            if (hacks::tf2::NavBot::task::current_task == hacks::tf2::NavBot::task::outofbounds)
            {
                if (nav::ReadyForCommands)
                    hacks::tf2::NavBot::task::current_task = hacks::tf2::NavBot::task::none;
                else
                    return;
            }
            if (GetMvmCredits() <= 250)
                return;
            if (CE_BAD(LOCAL_E))
                return;
            if (!buy_upgrade.check(5000))
                return;
            std::vector<Posinfo> potential_spots{};

            for (auto &i : spot_list)
            {
                if (lvlname.find(i.lvlname) != lvlname.npos)
                    potential_spots.push_back({ i.x, i.y, i.z, lvlname });
            }
            Posinfo best_spot{};
            float best_score = FLT_MAX;
            for (auto &i : potential_spots)
            {
                Vector pos  = { i.x, i.y, 0.0f };
                float score = pos.DistTo(LOCAL_E->m_vecOrigin());
                if (score < best_score)
                {
                    best_spot  = i;
                    best_score = score;
                }
            }
            if (GetMvmCredits() >= 400 || Vector{ best_spot.x, best_spot.y, best_spot.z }.DistTo(LOCAL_E->m_vecOrigin()) <= 500.0f)
            {
                NavUpgrade();
                buy_upgrade.update();
            }
        },
        "mvm_upgrade_createmove");
    EC::Register(
        EC::LevelShutdown, []() { inited_upgrades = false; }, "mvmupgrades_levelshutdown");
    spot_list.push_back(Posinfo(851.0f, -2509.0f, 577.0f, "mvm_coaltown"));
    spot_list.push_back(Posinfo(935.0f, -2626.0f, 577.0f, "mvm_bigrock"));
    spot_list.push_back(Posinfo(-885, -2229, 545, "mvm_decoy"));
    spot_list.push_back(Posinfo(851, -2509, 577, "mvm_ghost_town"));
    spot_list.push_back(Posinfo(-625, 2273, -95, "mvm_mannhatten"));
    spot_list.push_back(Posinfo(517, -2599, 450, "mvm_mannworks"));
    spot_list.push_back(Posinfo(-1346, 625, -102, "mvm_rottenburg"));
});

void MvM_Autoupgrade(KeyValues *event)
{
    if (!isHackActive())
        return;
    if (!*mvm_autoupgrade)
        return;
    std::string name = std::string(event->GetName());
    if (!name.compare("MVM_Upgrade"))
    {
        KeyValues *new_key = new KeyValues("upgrade");
        KeyValues *key     = event->FindKey("upgrade", false);
        if (!key)
        {
            new_key->SetInt("itemslot", 0);
            new_key->SetInt("upgrade", 0);
            new_key->SetInt("count", 1);
            event->AddSubKey(new_key);
            key = event->FindKey("upgrade", false);
        }
        else
            delete new_key;
        if (key)
        {

            auto upgrade = PickUpgrade();
            if (upgrade.id == -1)
                return;
            key->SetInt("itemslot", 0);
            key->SetInt("upgrade", upgrade.id);
            key->SetInt("count", 1);
        }
        else
            logging::Info("Key process failed!");
    }
    if (!name.compare("MvM_UpgradesBegin"))
    {
        logging::Info("Sent Upgrades");
        run = true;
        run_delay.update();
    }
}
*/
void SendNetMsg(INetMessage &msg)
{
    /*
    if (!strcmp(msg.GetName(), "clc_CmdKeyValues"))
    {
        if ((KeyValues *) (((unsigned *) &msg)[4]))
            MvM_Autoupgrade((KeyValues *) (((unsigned *) &msg)[4]));
    }*/
}
/*
class CatBotEventListener : public IGameEventListener2
{
    void FireGameEvent(IGameEvent *event) override
    {

        int killer_id = GetPlayerForUserID(event->GetInt("attacker"));
        int victim_id = GetPlayerForUserID(event->GetInt("userid"));

        if (victim_id == g_IEngine->GetLocalPlayer())
        {
            on_killed_by(killer_id);
            return;
        }
    }
};

CatBotEventListener &listener()
{
    static CatBotEventListener object{};
    return object;
}*/

class CatBotEventListener2 : public IGameEventListener2
{
    void FireGameEvent(IGameEvent *) override
    {
        // vote for current map if catbot mode and autovote is on
        if (catbotmode && autovote_map)
            g_IEngine->ServerCmd("next_map_vote 0");
    }
};

CatBotEventListener2 &listener2()
{
    static CatBotEventListener2 object{};
    return object;
}

Timer timer_votekicks{};
static Timer timer_catbot_list{};
static Timer timer_abandon{};

static int count_ipc = 0;
static std::vector<unsigned> ipc_list{ 0 };

static bool waiting_for_quit_bool{ false };
static Timer waiting_for_quit_timer{};

static std::vector<unsigned> ipc_blacklist{};

#if ENABLE_IPC
void update_ipc_data(ipc::user_data_s &data)
{
    data.ingame.bot_count = count_ipc;
}
#endif

Timer level_init_timer{};

Timer micspam_on_timer{};
Timer micspam_off_timer{};
static bool patched_report;
static std::atomic_bool can_report = false;
static std::vector<unsigned> to_report;
void reportall()
{
    can_report = false;
    if (!patched_report)
    {
        static BytePatch patch(gSignatures.GetClientSignature, "73 ? 80 7D ? ? 74 ? F3 0F 10 0D", 0x2F, { 0x89, 0xe0 });
        patch.Patch();
        patched_report = true;
    }
    for (auto const &ent: entity_cache::player_cache)
    {
       
        // We only want a nullptr check since dormant entities are still on the
        // server
        if (!ent)
            continue;

        // Pointer comparison is fine
        if (ent == LOCAL_E)
            continue;
        player_info_s info;
        if (GetPlayerInfo(ent->m_IDX, &info) && info.friendsID)
        {
            if (!player_tools::shouldTargetSteamId(info.friendsID))
                continue;
            to_report.push_back(info.friendsID);
        }
    }
    can_report = true;
}

CatCommand report("report_all", "Report all players", []() { reportall(); });
CatCommand report_uid("report_steamid", "Report with steamid",
                      [](const CCommand &args)
                      {
                          if (args.ArgC() < 2)
                              return;
                          unsigned steamid = 0;
                          try
                          {
                              steamid = std::stoi(args.Arg(1));
                          }
                          catch (const std::invalid_argument &)
                          {
                              logging::Info("Report machine broke");
                              return;
                          }
                          if (!steamid)
                          {
                              logging::Info("Report machine broke");
                              return;
                          }
                          typedef uint64_t (*ReportPlayer_t)(uint64_t, int);
                          static uintptr_t addr1                = gSignatures.GetClientSignature("55 89 E5 57 56 53 81 EC ? ? ? ? 8B 5D ? 8B 7D ? 89 D8");
                          static ReportPlayer_t ReportPlayer_fn = ReportPlayer_t(addr1);
                          if (!addr1)
                              return;
                          CSteamID id(steamid, EUniverse::k_EUniversePublic, EAccountType::k_EAccountTypeIndividual);
                          ReportPlayer_fn(id.ConvertToUint64(), 1);
                      });

Timer crouchcdr{};
void smart_crouch()
{
    if (g_Settings.bInvalid)
        return;
    if (!current_user_cmd)
        return;
    if (*always_crouch)
    {
        current_user_cmd->buttons |= IN_DUCK;
        if (crouchcdr.test_and_set(10000))
            current_user_cmd->buttons &= ~IN_DUCK;
        return;
    }
    bool foundtar      = false;
    static bool crouch = false;
    if (crouchcdr.test_and_set(2000))
    {
        for (auto const &ent: entity_cache::player_cache)
        {
            if (CE_BAD(ent) || ent->m_Type() != ENTITY_PLAYER || ent->m_iTeam() == LOCAL_E->m_iTeam() || !(ent->hitboxes.GetHitbox(0)) || !(ent->m_bAlivePlayer()) || !player_tools::shouldTarget(ent))
                continue;
            bool failedvis = false;
            for (int j = 0; j < 18; j++)
                if (IsVectorVisible(g_pLocalPlayer->v_Eye, ent->hitboxes.GetHitbox(j)->center))
                    failedvis = true;
            if (failedvis)
                continue;
            for (int j = 0; j < 18; j++)
            {
                if (!LOCAL_E->hitboxes.GetHitbox(j))
                    continue;
                // Check if they see my hitboxes
                if (!IsVectorVisible(ent->hitboxes.GetHitbox(0)->center, LOCAL_E->hitboxes.GetHitbox(j)->center) && !IsVectorVisible(ent->hitboxes.GetHitbox(0)->center, LOCAL_E->hitboxes.GetHitbox(j)->min) && !IsVectorVisible(ent->hitboxes.GetHitbox(0)->center, LOCAL_E->hitboxes.GetHitbox(j)->max))
                    continue;
                foundtar = true;
                crouch   = true;
            }
        }
        if (!foundtar && crouch)
            crouch = false;
    }
    if (crouch)
        current_user_cmd->buttons |= IN_DUCK;
}

CatCommand print_ammo("debug_print_ammo", "debug",
                      []()
                      {
                          if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || CE_BAD(LOCAL_W))
                              return;
                          logging::Info("Current slot: %d", re::C_BaseCombatWeapon::GetSlot(RAW_ENT(LOCAL_W)));
                          for (int i = 0; i < 10; ++i)
                              logging::Info("Ammo Table %d: %d", i, CE_INT(LOCAL_E, netvar.m_iAmmo + i * 4));
                      });
static Timer disguise{};
static Timer report_timer{};
static std::string health = "Health: 0/0";
static std::string ammo   = "Ammo: 0/0";
static int max_ammo;
static CachedEntity *local_w;
// TODO: add more stuffs
static void cm()
{
    if (!*catbotmode)
        return;

    if (CE_GOOD(LOCAL_E))
    {
        if (LOCAL_W != local_w)
        {
            local_w  = LOCAL_W;
            max_ammo = 0;
        }
        float max_hp  = g_pPlayerResource->GetMaxHealth(LOCAL_E);
        float curr_hp = CE_INT(LOCAL_E, netvar.iHealth);
        int ammo0     = CE_INT(LOCAL_E, netvar.m_iClip2);
        int ammo2     = CE_INT(LOCAL_E, netvar.m_iClip1);
        if (ammo0 + ammo2 > max_ammo)
            max_ammo = ammo0 + ammo2;
        health = format("Health: ", curr_hp, "/", max_hp);
        ammo   = format("Ammo: ", ammo0 + ammo2, "/", max_ammo);
    }
    if (g_Settings.bInvalid)
        return;

    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W))
        return;

    if (*auto_crouch)
        smart_crouch();

    //
    static const int classes[3]{ tf_spy, tf_sniper, tf_pyro };
    if (*auto_disguise && g_pPlayerResource->GetClass(LOCAL_E) == tf_spy && !IsPlayerDisguised(LOCAL_E) && disguise.test_and_set(3000))
    {
        int teamtodisguise = (LOCAL_E->m_iTeam() == TEAM_RED) ? TEAM_RED - 1 : TEAM_BLU - 1;
        int classtojoin    = classes[rand() % 3];
        g_IEngine->ClientCmd_Unrestricted(format("disguise ", classtojoin, " ", teamtodisguise).c_str());
    }
    if (*autoReport && report_timer.test_and_set(60000))
        reportall();
}

static Timer unstuck{};
static int unstucks;
static Timer report_timer2{};
void update()
{
    if (g_Settings.bInvalid)
        return;

    if (can_report)
    {
        typedef uint64_t (*ReportPlayer_t)(uint64_t, int);
        static uintptr_t addr1                = gSignatures.GetClientSignature("55 89 E5 57 56 53 81 EC ? ? ? ? 8B 5D ? 8B 7D ? 89 D8");
        static ReportPlayer_t ReportPlayer_fn = ReportPlayer_t(addr1);
        if (!addr1)
            return;
        if (report_timer2.test_and_set(400))
        {
            if (to_report.empty())
                can_report = false;
            else
            {
                auto rep = to_report.back();
                to_report.pop_back();
                CSteamID id(rep, EUniverse::k_EUniversePublic, EAccountType::k_EAccountTypeIndividual);
                ReportPlayer_fn(id.ConvertToUint64(), 1);
            }
        }
    }
    if (!catbotmode)
        return;

    if (CE_BAD(LOCAL_E))
        return;

    if (LOCAL_E->m_bAlivePlayer())
    {
        unstuck.update();
        unstucks = 0;
    }
    if (unstuck.test_and_set(10000))
    {
        unstucks++;
        // Send menuclosed to tell the server that we want to respawn
        hack::command_stack().push("menuclosed");
        // If that didnt work, force pick a team and class
        if (unstucks > 3)
            hack::command_stack().push("autoteam; join_class sniper");
    }

    if (micspam)
    {
        if (micspam_on && micspam_on_timer.test_and_set(*micspam_on * 1000))
            g_IEngine->ClientCmd_Unrestricted("+voicerecord");
        if (micspam_off && micspam_off_timer.test_and_set(*micspam_off * 1000))
            g_IEngine->ClientCmd_Unrestricted("-voicerecord");
    }

    if (random_votekicks && timer_votekicks.test_and_set(5000))
        do_random_votekick();
    if (timer_abandon.test_and_set(2000) && level_init_timer.check(13000))
    {
        count_ipc = 0;
        ipc_list.clear();
        int count_total = 0;

        for (auto const &ent: entity_cache::player_cache)
        {
            int i = ent->m_IDX;
            if (g_IEntityList->GetClientEntity(i))
                ++count_total;
            else
                continue;

            player_info_s info{};
            if (!GetPlayerInfo(i, &info))
                continue;
            if (playerlist::AccessData(info.friendsID).state == playerlist::k_EState::CAT)
                --count_total;

            if (playerlist::AccessData(info.friendsID).state == playerlist::k_EState::IPC || playerlist::AccessData(info.friendsID).state == playerlist::k_EState::TEXTMODE)
            {
                ipc_list.push_back(info.friendsID);
                ++count_ipc;
            }
        }

        if (abandon_if_ipc_bots_gte)
        {
            if (count_ipc >= int(abandon_if_ipc_bots_gte))
            {
                // Store local IPC Id and assign to the quit_id variable for later comparisions
                unsigned local_ipcid = ipc::peer->client_id;
                unsigned quit_id     = local_ipcid;

                // Iterate all the players marked as bot
                for (auto &id : ipc_list)
                {
                    // We already know we shouldn't quit, so just break out of the loop
                    if (quit_id < local_ipcid)
                        break;

                    // Reduce code size
                    auto &peer_mem = ipc::peer->memory;

                    // Iterate all ipc peers
                    for (unsigned i = 0; i < cat_ipc::max_peers; ++i)
                    {
                        // If that ipc peer is alive and in has the steamid of that player
                        if (!peer_mem->peer_data[i].free && peer_mem->peer_user_data[i].friendid == id)
                        {
                            // Check against blacklist
                            if (std::find(ipc_blacklist.begin(), ipc_blacklist.end(), i) != ipc_blacklist.end())
                                continue;

                            // Found someone with a lower ipc id
                            if (i < local_ipcid)
                            {
                                quit_id = i;
                                break;
                            }
                        }
                    }
                }
                // Only quit if you are the player with the lowest ipc id
                if (quit_id == local_ipcid)
                {
                    // Clear blacklist related stuff
                    waiting_for_quit_bool = false;
                    ipc_blacklist.clear();

                    logging::Info("Abandoning because there are %d local players "
                                  "in game, and abandon_if_ipc_bots_gte is %d.",
                                  count_ipc, int(abandon_if_ipc_bots_gte));
                    tfmm::abandon();
                    return;
                }
                else
                {
                    if (!waiting_for_quit_bool)
                    {
                        // Waiting for that ipc id to quit, we use this timer in order to blacklist
                        // ipc peers which refuse to quit for some reason
                        waiting_for_quit_bool = true;
                        waiting_for_quit_timer.update();
                    }
                    else
                    {
                        // IPC peer isn't leaving, blacklist for now
                        if (waiting_for_quit_timer.test_and_set(10000))
                        {
                            ipc_blacklist.push_back(quit_id);
                            waiting_for_quit_bool = false;
                        }
                    }
                }
            }
            else
            {
                // Reset Bool because no reason to quit
                waiting_for_quit_bool = false;
                ipc_blacklist.clear();
            }
        }
        if (abandon_if_humans_lte)
        {
            if (count_total - count_ipc <= int(abandon_if_humans_lte))
            {
                logging::Info("Abandoning because there are %d non-bots in "
                              "game, and abandon_if_humans_lte is %d.",
                              count_total - count_ipc, int(abandon_if_humans_lte));
                tfmm::abandon();
                return;
            }
        }
        if (abandon_if_players_lte)
        {
            if (count_total <= int(abandon_if_players_lte))
            {
                logging::Info("Abandoning because there are %d total players "
                              "in game, and abandon_if_players_lte is %d.",
                              count_total, int(abandon_if_players_lte));
                tfmm::abandon();
                return;
            }
        }
    }
}

void init()
{
    // g_IEventManager2->AddListener(&listener(), "player_death", false);
    g_IEventManager2->AddListener(&listener2(), "vote_maps_changed", false);
}

void level_init()
{
    deaths = 0;
    level_init_timer.update();
}

void shutdown()
{
    // g_IEventManager2->RemoveListener(&listener());
    g_IEventManager2->RemoveListener(&listener2());
}

#if ENABLE_VISUALS
static void draw()
{
    if (!catbotmode || !anti_motd)
        return;
    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer())
        return;
    AddCenterString(health, colors::green);
    AddCenterString(ammo, colors::yellow);
}
#endif

static InitRoutine runinit(
    []()
    {
        EC::Register(EC::CreateMove, cm, "cm_catbot", EC::average);
        EC::Register(EC::CreateMove, update, "cm2_catbot", EC::average);
        EC::Register(EC::LevelInit, level_init, "levelinit_catbot", EC::average);
        EC::Register(EC::Shutdown, shutdown, "shutdown_catbot", EC::average);
#if ENABLE_VISUALS
        EC::Register(EC::Draw, draw, "draw_catbot", EC::average);
#endif
        init();
    });
} // namespace hacks::shared::catbot
