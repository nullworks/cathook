/*
    This file is part of Cathook.

    Cathook is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cathook is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cathook. If not, see <https://www.gnu.org/licenses/>.
*/

// Codeowners: aUniqueUser

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "common.hpp"

namespace hacks::tf2::autoitem
{

static settings::Boolean enable{ "auto-item.enable", "false" };
static settings::Int interval{ "auto-item.time", "30000" };

// stock by default
static settings::Boolean weapons{ "auto-item.weapons", "false" };
static settings::String primary{ "auto-item.weapons.primary", "-1" };
static settings::String secondary{ "auto-item.weapons.secondary", "-1" };
static settings::String melee{ "auto-item.weapons.melee", "-1" };

// ben cat hats by default
static settings::Boolean hats{ "auto-item.hats", "false" };
static settings::Int hat1{ "auto-item.hats.1", "940" };
static settings::Int hat2{ "auto-item.hats.2", "941" };
static settings::Int hat3{ "auto-item.hats.3", "302" };

static settings::Boolean debug{ "auto-item.debug", "false" };

#define Debug(...) \
    if (*debug)    \
    logging::Info("AutoItem.cpp: " __VA_ARGS__)

#if ENABLE_TEXTMODE
static settings::Boolean autoNoisemaker{ "misc.auto-noisemaker", "true" };
#else
static settings::Boolean autoNoisemaker{ "misc.auto-noisemaker", "false" };
#endif

// 536 is Birthday noisemaker
// 673 is Christmas noisemaker
static int noisemaker_id = 536;

struct AchivementItem
{
    int achievement_id;
    std::string name;
};

// A map that allows us to map item ids to achievement names and achievement ids
static std::unordered_map<int /*item_id*/, AchivementItem> ach_items;
static std::array<std::vector<std::string>, 3> craft_groups;

bool checkAchMgr()
{
    if (!g_IAchievementMgr)
    {
        g_IAchievementMgr = g_IEngine->GetAchievementMgr();
        if (!g_IAchievementMgr)
            return false;
    }
    return true;
}

void Lock()
{
    if (!checkAchMgr())
        return;
    g_ISteamUserStats->RequestCurrentStats();
    for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++)
    {
        g_ISteamUserStats->ClearAchievement(g_IAchievementMgr->GetAchievementByIndex(i)->GetName());
    }
    g_ISteamUserStats->StoreStats();
    g_ISteamUserStats->RequestCurrentStats();
}

void Unlock()
{
    if (!checkAchMgr())
        return;
    for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++)
    {
        g_IAchievementMgr->AwardAchievement(g_IAchievementMgr->GetAchievementByIndex(i)->GetAchievementID());
    }
}

void unlockSingle(int achID)
{
    if (!checkAchMgr())
        return;
    auto *ach = reinterpret_cast<IAchievement *>(g_IAchievementMgr->GetAchievementByID(achID));
    if (ach)
    {
        g_IAchievementMgr->AwardAchievement(achID);
    }
}

AchivementItem *isAchItem(int id)
{
    auto val = ach_items.find(id);
    return val != ach_items.end() ? &val->second : nullptr;
}

void getItem(int id, bool rent = true)
{
    Debug("Trying to get item, %i", id);
    auto index = isAchItem(id);
    if (index)
    {
        unlockSingle(index->achievement_id);
        g_IEngine->ClientCmd_Unrestricted("cl_trigger_first_notification");
    }
    else if (rent)
        Rent(id);
    else
        Debug("Failed to get item %i", id);
}

static bool equipItem(int clazz, int slot, int id, bool get = true, bool allowRent = false)
{
    auto invmng = re::CTFInventoryManager::GTFInventoryManager();
    auto inv    = invmng->GTFPlayerInventory();

    // Slot correction for spy
    if (g_pLocalPlayer->clazz == tf_spy)
    {
        // Secondary -> Cloak
        if (slot == 1)
            slot = 6;
    }

    if (id == -1)
        return invmng->EquipItemInLoadout(clazz, slot, -1);

    if (get)
    {
        auto item_view = inv->GetFirstItemOfItemDef(id);
        if (!item_view)
        {
            getItem(id, allowRent);
            return false;
        }
    }
    auto item_view = inv->GetFirstItemOfItemDef(id);
    if (item_view)
        return invmng->EquipItemInLoadout(clazz, slot, item_view->UUID());

    return false;
}

static bool use_fallback       = false;
static int first_item_attempts = 0;
void getAndEquipWeapon(std::string str, int clazz, int slot)
{
    auto invmng = re::CTFInventoryManager::GTFInventoryManager();
    auto inv    = invmng->GTFPlayerInventory();

    if (str == "-1")
    {
        equipItem(clazz, slot, -1, false, false);
        return;
    }

    // Single item, rent or simply get via achievement, use fallback if needed and specified.
    if (str.find(',') == std::string::npos && str.find(';') == std::string::npos)
    {
        std::vector<std::string> ids_split_str;
        std::vector<int> ids_split;
        try
        {
            if (str.find('/') != std::string::npos)
            {
                boost::split(ids_split_str, str, boost::is_any_of("/"));
                for (auto &id : ids_split_str)
                {
                    ids_split.emplace_back(std::stoi(id));
                }
                use_fallback = true;
            }
            else
                ids_split.emplace_back(std::stoi(str));
        }
        catch (std::invalid_argument &e)
        {
            Debug("invalid_argument error making ids_split vector.");
            return;
        }

        auto item_view = inv->GetFirstItemOfItemDef(ids_split.at(0));

        // Try the first item 3 times before moving to fallback
        if (use_fallback)
        {
            if (!item_view && first_item_attempts >= 3)
                equipItem(clazz, slot, ids_split.at(1), true, true);
            else
            {
                equipItem(clazz, slot, ids_split.at(0), true, true);
                first_item_attempts++;
            }
        }
        else
            equipItem(clazz, slot, ids_split.at(0), true, true);
    }
    else
    {
        // Using auto-craft
        int result;
        size_t loc = str.find('-');
        if (loc != std::string::npos)
        {
            try
            {
                result = std::stoi(str.substr(loc + 1, str.length()));
            }
            catch (std::invalid_argument &e)
            {
                Debug("invalid_argument error making result integer.");
                return;
            }

            auto item_view = inv->GetFirstItemOfItemDef(result);
            if (item_view)
            {
                equipItem(clazz, slot, result, false, false);
                return;
            }

            std::vector<std::string> ids_rec_str;
            std::vector<int> ids_rec;

            for (auto &group_str : craft_groups[slot])
            {
                Debug("Crafting group: %s", group_str.c_str());
                ids_rec.clear();
                ids_rec_str.clear();

                // Last group should be the just the result, skip it
                if (group_str == std::to_string(result))
                    continue;

                // Split this crafting group into IDs
                boost::split(ids_rec_str, group_str, boost::is_any_of(","));

                try
                {
                    // Convert to ints
                    for (auto &id : ids_rec_str)
                    {
                        ids_rec.emplace_back(std::stoi(id));
                    }
                }
                catch (std::invalid_argument &e)
                {
                    Debug("invalid_argument error making rec ids vector.");
                    return;
                }

                // See if we have the requirements to perform this craft, if not try to get them.
                size_t rec_req_amount_have = 0;
                for (auto &id : ids_rec)
                {
                    // For now we should just assume the user has
                    // provided a way for getting needed scrap
                    if (id == 5000 || id == 5001 || id == 5002)
                    {
                        rec_req_amount_have++;
                        continue;
                    }

                    // In this loop id is a item id of 1 part of the crafting group
                    auto item_view = inv->GetFirstItemOfItemDef(id);
                    if (item_view)
                        rec_req_amount_have++;
                    else
                    {
                        auto index = isAchItem(id);
                        if (index)
                        {
                            if (!checkAchMgr())
                                return;
                            if (g_IAchievementMgr->HasAchieved(index->name.c_str()))
                            {
                                Debug("Cant get specified ach item %i because it has already been unlocked! moving to next item.", id);
                                // User's fault if they put a ach item they unlocked and have used for crafting already/deleted.
                                rec_req_amount_have++;
                                continue;
                            }
                            Debug("Getting ach item %i, required for this craft.", id);
                            getItem(id, false);
                            return;
                        }
                        else
                        {
                            Debug("Missing required crafting materials! %i is NOT a ach item!", id);
                            return;
                        }
                    }
                }
                // Only attempt crafting if we have the correct amount of items.
                if (rec_req_amount_have == ids_rec.size())
                {
                    Craft(ids_rec);
                    g_IEngine->ClientCmd_Unrestricted("cl_trigger_first_notification");
                }
            }
        }
        else
        {
            Debug("No result item, can't auto-craft.");
            return;
        }
    }
}

static Timer t{};
void CreateMove()
{
    if (!enable || CE_BAD(LOCAL_E) || !t.test_and_set(*interval))
        return;

    int clazz   = g_pLocalPlayer->clazz;
    auto invmng = re::CTFInventoryManager::GTFInventoryManager();
    auto inv    = invmng->GTFPlayerInventory();

    // Only run if we are playing a valid class
    if (clazz != 0)
    {
        if (weapons)
        {
            getAndEquipWeapon(*primary, clazz, 0);
            getAndEquipWeapon(*secondary, clazz, 1);
            getAndEquipWeapon(*melee, clazz, 2);
        }
        if (hats)
        {
            static int offset         = 0;
            const static int slots[3] = { 7, 8, 10 };

            equipItem(clazz, slots[offset], *hat1);
            equipItem(clazz, slots[(offset + 1) % 3], *hat2);
            equipItem(clazz, slots[(offset + 2) % 3], *hat3);
            offset = (offset + 1) % 3;
        }
        if (autoNoisemaker && inv->GetFirstItemOfItemDef(noisemaker_id))
        {
            equipItem(clazz, 9, noisemaker_id, false, false);
        }
    }
}

CatCommand dump_achievement("achievement_dump", "Dump achievements to file (development)", []() {
    if (!checkAchMgr())
        return;
    std::ofstream out("/tmp/cathook_achievements.txt", std::ios::out);
    if (out.bad())
        return;
    for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++)
    {
        out << '[' << i << "] " << g_IAchievementMgr->GetAchievementByIndex(i)->GetName() << ' ' << g_IAchievementMgr->GetAchievementByIndex(i)->GetAchievementID() << "\n";
    }
    out.close();
});

CatCommand unlock_single("achievement_unlock_single", "Unlocks single achievement by ID", [](const CCommand &args) {
    if (!checkAchMgr())
        return;
    char *out = nullptr;
    int id    = strtol(args.Arg(1), &out, 10);
    if (out == args.Arg(1))
    {
        logging::Info("NaN achievement ID!");
        return;
    }
    unlockSingle(id);
});

// For some reason it SEGV's when I try to GetAchievementByID();
CatCommand lock_single("achievement_lock_single", "Locks single achievement by INDEX!", [](const CCommand &args) {
    if (!checkAchMgr())
        return;
    if (args.ArgC() < 2)
    {
        logging::Info("Actually provide an index");
        return;
    }
    int id;
    try
    {
        id = std::stoi(args.Arg(1));
    }
    catch (const std::invalid_argument &)
    {
        logging::Info("Bad achievement ID!");
        return;
    }
    auto *ach = reinterpret_cast<IAchievement *>(g_IAchievementMgr->GetAchievementByID(id));

    int index = -1;
    if (ach)
        for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++)
        {
            auto ach2 = g_IAchievementMgr->GetAchievementByIndex(i);
            if (ach2->GetAchievementID() == id)
            {
                index = i;
                break;
            }
        }
    if (ach && index != -1)
    {
        g_ISteamUserStats->RequestCurrentStats();
        auto ach = g_IAchievementMgr->GetAchievementByIndex(index);
        g_ISteamUserStats->ClearAchievement(ach->GetName());
        g_ISteamUserStats->StoreStats();
        g_ISteamUserStats->RequestCurrentStats();
    }
});

CatCommand rent_item("rent_item", "testrun a item by ID", [](const CCommand &args) {
    char *out = nullptr;
    int id    = strtol(args.Arg(1), &out, 10);
    if (out == args.Arg(1))
    {
        logging::Info("Bad item ID!");
        return;
    }
    Rent(id);
});

CatCommand lock("achievement_lock", "Lock all achievements", Lock);
CatCommand unlock("achievement_unlock", "Unlock all achievements", Unlock);

void rvarCallback(std::string after, int idx)
{
    craft_groups[idx].clear();
    boost::split(craft_groups[idx], after, boost::is_any_of(";-"));
}

static InitRoutine init([]() {
    primary.installChangeCallback([](settings::VariableBase<std::string> &, std::string after) { rvarCallback(after, 0); });
    secondary.installChangeCallback([](settings::VariableBase<std::string> &, std::string after) { rvarCallback(after, 1); });
    melee.installChangeCallback([](settings::VariableBase<std::string> &, std::string after) { rvarCallback(after, 2); });

    EC::Register(EC::CreateMove, CreateMove, "autoitem_cm");

    std::time_t theTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm *aTime      = std::localtime(&theTime);

    int day   = aTime->tm_mday;
    int month = aTime->tm_mon + 1; // Month is 0 - 11, add 1 to get a jan-dec 1-12 concept

    // We only want to Use the christmas noisemaker around christmas time, let's use 12th of december+ til 12th of january
    if ((month == 12 && day >= 12) || (month == 1 && day <= 12))
        noisemaker_id = 673;

    /* haha operator[] go brrrrrr */
    ach_items[45]   = { 1036, "TF_SCOUT_ACHIEVE_PROGRESS1" };         // Force-A-Nature - Scout Milestone 1
    ach_items[44]   = { 1037, "TF_SCOUT_ACHIEVE_PROGRESS2" };         // Sandman - Scout Milestone 2
    ach_items[46]   = { 1038, "TF_SCOUT_ACHIEVE_PROGRESS3" };         // Bonk! Atomic Punch - Scout Milestone 3
    ach_items[128]  = { 1236, "TF_SOLDIER_ACHIEVE_PROGRESS1" };       // Equalizer - Soldier Milestone 1
    ach_items[127]  = { 1237, "TF_SOLDIER_ACHIEVE_PROGRESS2" };       // Direct Hit - Soldier Milestone 2
    ach_items[129]  = { 1238, "TF_SOLDIER_ACHIEVE_PROGRESS3" };       // Buff Banner - Soldier Milestone 3
    ach_items[39]   = { 1637, "TF_PYRO_ACHIEVE_PROGRESS1" };          // Flare Gun - Pyro Milestone 1
    ach_items[40]   = { 1638, "TF_PYRO_ACHIEVE_PROGRESS2" };          // Backburner - Pyro Milestone 2
    ach_items[38]   = { 1639, "TF_PYRO_ACHIEVE_PROGRESS3" };          // Axtinguisher - Pyro Milestone 3
    ach_items[131]  = { 1336, "TF_DEMOMAN_ACHIEVE_PROGRESS1" };       // Chargin' Targe - Demoman Milestone 1
    ach_items[132]  = { 1337, "TF_DEMOMAN_ACHIEVE_PROGRESS2" };       // Eyelander - Demoman Milestone 2
    ach_items[130]  = { 1338, "TF_DEMOMAN_ACHIEVE_PROGRESS3" };       // Scottish Resistance - Demoman Milestone 3
    ach_items[42]   = { 1537, "TF_HEAVY_ACHIEVE_PROGRESS1" };         // Sandvich - Heavy Milestone 1
    ach_items[41]   = { 1538, "TF_HEAVY_ACHIEVE_PROGRESS2" };         // Natascha - Heavy Milestone 2
    ach_items[43]   = { 1539, "TF_HEAVY_ACHIEVE_PROGRESS3" };         // Killing Gloves of Boxing - Heavy Milestone 3
    ach_items[141]  = { 1801, "TF_ENGINEER_ACHIEVE_PROGRESS1" };      // Frontier Justice - Engineer Milestone 1
    ach_items[142]  = { 1802, "TF_ENGINEER_ACHIEVE_PROGRESS2" };      // Gunslinger - Engineer Milestone 2
    ach_items[140]  = { 1803, "TF_ENGINEER_ACHIEVE_PROGRESS3" };      // Wrangler - Engineer Milestone 3
    ach_items[36]   = { 1437, "TF_MEDIC_ACHIEVE_PROGRESS1" };         // Blutsauger - Medic Milestone 1
    ach_items[35]   = { 1438, "TF_MEDIC_ACHIEVE_PROGRESS2" };         // Kritzkrieg - Medic Milestone 2
    ach_items[37]   = { 1439, "TF_MEDIC_ACHIEVE_PROGRESS3" };         // Ubersaw - Medic Milestone 3
    ach_items[56]   = { 1136, "TF_SNIPER_ACHIEVE_PROGRESS1" };        // Huntsman - Sniper Milestone 1
    ach_items[58]   = { 1137, "TF_SNIPER_ACHIEVE_PROGRESS2" };        // Jarate - Sniper Milestone 2
    ach_items[57]   = { 1138, "TF_SNIPER_ACHIEVE_PROGRESS3" };        // Razorback - Sniper Milestone 3
    ach_items[61]   = { 1735, "TF_SPY_ACHIEVE_PROGRESS1" };           // Ambassador - Spy Milestone 1
    ach_items[60]   = { 1736, "TF_SPY_ACHIEVE_PROGRESS2" };           // Cloak and Dagger - Spy Milestone 2
    ach_items[59]   = { 1737, "TF_SPY_ACHIEVE_PROGRESS3" };           // Dead Ringer - Spy Milestone 3
    ach_items[1123] = { 1928, "TF_HALLOWEEN_DOOMSDAY_MILESTONE" };    // Necro Smasher - Carnival of Carnage: Step Right Up
    ach_items[940]  = { 1902, "TF_HALLOWEEN_DOMINATE_FOR_HAT" };      // Ghostly Gibus - Ghastly Gibus Grab
    ach_items[115]  = { 1901, "TF_HALLOWEEN_COLLECT_PUMPKINS" };      // Mildly Disturbing Halloween Mask - Candy Coroner
    ach_items[278]  = { 1906, "TF_HALLOWEEN_BOSS_KILL" };             // Horseless Headless Horsemann's Head - Sleepy Holl0WND
    ach_items[302]  = { 2006, "TF_REPLAY_YOUTUBE_VIEWS_TIER2" };      // Frontline Field Recorder - Local Cinema Star
    ach_items[581]  = { 1910, "TF_HALLOWEEN_EYEBOSS_KILL" };          // MONOCULUS! - Optical Defusion
    ach_items[668]  = { 2212, "TF_MAPS_FOUNDRY_ACHIEVE_PROGRESS1" };  // Full Head Of Steam - Foundry Milestone
    ach_items[756]  = { 2412, "TF_MAPS_DOOMSDAY_ACHIEVE_PROGRESS1" }; // Gentle Munitionne of Leisure - Doomsday Milestone
    ach_items[941]  = { 1912, "TF_HALLOWEEN_MERASMUS_COLLECT_LOOT" }; // Skull Island Topper - A Lovely Vacation Spot
    ach_items[581]  = { 1911, "TF_HALLOWEEN_LOOT_ISLAND" };           // Bombinomicon - Dive Into a Good Book
    ach_items[744]  = { 156, "TF_DOMINATE_FOR_GOGGLES" };             // Pyrovision Goggles - A Fresh Pair of Eyes
    ach_items[1164] = { 167, "TF_PASS_TIME_GRIND" };                  // Civilian Grade JACK Hat - Jackpot!
    ach_items[1169] = { 166, "TF_PASS_TIME_HAT" };                    // Military Grade JACK Hat - Tune Merasmus's Multi-Dimensional Television
    ach_items[1170] = { 166, "TF_PASS_TIME_HAT" };                    // PASS Time Miniature Half JACK - Tune Merasmus's Multi-Dimensional Television
    ach_items[267]  = { 1909, "TF_HALLOWEEN_BOSS_KILL_MELEE" };       // Haunted Metal Scrap - Gored!
});
} // namespace hacks::tf2::autoitem
