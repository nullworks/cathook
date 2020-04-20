/*
 * Achievement.cpp
 *
 *  Created on: Jan 20, 2017
 *      Author: nullifiedcat
 */

#include <settings/Bool.hpp>
#include "Misc.hpp"
#include "common.hpp"

namespace hacks::tf2::achievement
{
static settings::Boolean safety{ "achievement.safety", "true" };

struct Autoequip_unlock_list
{
    std::string name;
    int achievement_id;
    int item_id;
    tf_class player_class;
    int slot;
    Autoequip_unlock_list(std::string _name, int _ach_id, int _item_id, tf_class _player_class, int _slot)
    {
        name           = _name;
        achievement_id = _ach_id;
        item_id        = _item_id;
        player_class   = _player_class;
        slot           = _slot;
    }
};

// Keep track of possible achievement items
static std::vector<Autoequip_unlock_list> primary, secondary, melee, pda2;
// Keep track of stock items
static std::array<bool, 4> equip_stock;

static settings::Int equip_primary{ "achievement.equip-primary", "0" };
static settings::Int equip_secondary{ "achievement.equip-secondary", "0" };
static settings::Int equip_melee{ "achievement.equip-melee", "0" };
static settings::Int equip_pda2{ "achievement.equip-pda2", "0" };
static settings::Boolean hat_troll{ "misc.nohatsforyou", "false" };
#if ENABLE_TEXTMODE
static settings::Boolean auto_noisemaker{ "misc.auto-noisemaker", "true" };
#else
static settings::Boolean auto_noisemaker{ "misc.auto-noisemaker", "false" };
#endif

bool checkachmngr()
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
    if (!checkachmngr())
        return;
    if (safety)
    {
        ConColorMsg({ 255, 0, 0, 255 }, "Switch `cat set achievement.safety` to false before using any achievement commands!\n");
        return;
    }
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
    if (!checkachmngr())
        return;
    /*auto Invmng = re::CTFInventoryManager::GTFInventoryManager();
    auto Inv = re::CTFPlayerInventory::GTFPlayerInventory();
    auto Item = Inv->GetFirstItemOfItemDef(59);
    Invmng->EquipItemInLoadout(0, 0, (unsigned long long
    int)Item->uniqueid());*/
    if (safety)
    {
        ConColorMsg({ 255, 0, 0, 255 }, "Switch `cat set achievement.safety` to false before using any achievement commands!\n");
        return;
    }
    for (int i = 0; i < g_IAchievementMgr->GetAchievementCount(); i++)
    {
        g_IAchievementMgr->AwardAchievement(g_IAchievementMgr->GetAchievementByIndex(i)->GetAchievementID());
    }
}

CatCommand dump_achievement("achievement_dump", "Dump achievements to file (development)", []() {
    if (!checkachmngr())
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
    if (!checkachmngr())
        return;
    char *out = nullptr;
    int id    = strtol(args.Arg(1), &out, 10);
    if (out == args.Arg(1))
    {
        logging::Info("NaN achievement ID!");
        return;
    }
    IAchievement *ach = reinterpret_cast<IAchievement *>(g_IAchievementMgr->GetAchievementByID(id));
    if (ach)
    {
        g_IAchievementMgr->AwardAchievement(id);
    }
});
// For some reason it SEGV's when I try to GetAchievementByID();
CatCommand lock_single("achievement_lock_single", "Locks single achievement by INDEX!", [](const CCommand &args) {
    if (!checkachmngr())
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
    catch (std::invalid_argument)
    {
        logging::Info("NaN achievement ID!");
        return;
    }
    IAchievement *ach = reinterpret_cast<IAchievement *>(g_IAchievementMgr->GetAchievementByID(id));

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
CatCommand lock("achievement_lock", "Lock all achievements", Lock);
CatCommand unlock("achievement_unlock", "Unlock all achievements", Unlock);

static bool accept_notifs;
static bool equip_hats;
static bool equip;
std::vector<Autoequip_unlock_list> equip_queue;

void unlock_achievements_and_accept(std::vector<int> items)
{
    if (!checkachmngr())
        return;
    for (auto id : items)
    {
        IAchievement *ach = reinterpret_cast<IAchievement *>(g_IAchievementMgr->GetAchievementByID(id));
        if (ach)
        {
            g_IAchievementMgr->AwardAchievement(id);
        }
    }
    accept_notifs = true;
}

static CatCommand get_sniper_items("achievement_sniper", "Get all sniper achievement items", []() {
    static std::vector<int> sniper_items = { 1136, 1137, 1138 };
    unlock_achievements_and_accept(sniper_items);
});

static Timer accept_time{};
static Timer equip_noisemaker{};
static Timer cooldowm{};
static Timer cooldown_2{};
static CatCommand get_best_hats("achievement_cathats", "Get and equip the bencat hats", []() {
    static std::vector<int> bencat_hats = { 1902, 1912, 2006 };
    unlock_achievements_and_accept(bencat_hats);
    hacks::shared::misc::generate_schema();
    hacks::shared::misc::Schema_Reload();
    equip_hats = true;
});

struct queue_struct
{
    int clazz;
    int slot;
    unsigned long long uuid;
};
static std::deque<queue_struct> gc_queue;

bool equip_item(int clazz, int slot, int id)
{
    auto invmng = re::CTFInventoryManager::GTFInventoryManager();
    auto inv    = invmng->GTFPlayerInventory();
    // Request to equip a stock item
    if (id == -1)
    {
        // -1 in Unsigned long long would be the maximum value
        gc_queue.push_front({ clazz, slot, ULLONG_MAX });
        return true;
    }
    else
    {
        auto item_view = inv->GetFirstItemOfItemDef(id);
        if (item_view)
        {
            gc_queue.push_front({ clazz, slot, item_view->UUID() });
            return true;
        }
    }
    return false;
}
bool equip_action_item(int action_item, std::pair<int, int> classes)
{
    for (int i = classes.first; i <= classes.second; i++)
        if (!equip_item(i, 9, action_item))
            return false;
    return true;
}

bool equip_hats_fn(std::vector<int> hats, std::pair<int, int> classes)
{
    for (int i = classes.first; i <= classes.second; i++)
        // 7...8...10. Wait why is it 10 tf2? you make no sense
        if (!(equip_item(i, 7, hats[0]) && equip_item(i, 8, hats[1]) && equip_item(i, 10, hats[2])))
            return false;
    return true;
}

static std::vector<int> hat_list = { 302, 940, 941 };
// TNE didn't want me to put "epic_" infront of it :(
static Timer hat_steal_timer{};
static Timer gc_timer{};
// We need this incase a bot joins one class due to being stuck
// and then switches to the other while the stock equip is on
static std::array<Timer, 4> stock_equip_reset{};
// We also need a delay between each attempt to not spam gc
static Timer stock_equip_wait{};

void CreateMove()
{
    if (gc_queue.size() && gc_timer.test_and_set(3000))
    {
        for (int i = 0; i < 10 && gc_queue.size(); i++)
        {
            queue_struct item = gc_queue.at(gc_queue.size() - 1);
            auto invmng       = re::CTFInventoryManager::GTFInventoryManager();
            invmng->EquipItemInLoadout(item.clazz, item.slot, item.uuid);
            gc_queue.pop_back();
        }
    }
    if (CE_BAD(LOCAL_E))
        return;

    // Wait 500ms between each call
    if (LOCAL_E->m_bAlivePlayer() && stock_equip_wait.test_and_set(500))
        for (int i = 0; i < 4; i++)
        {
            // We should equip our stock weapons
            if (equip_stock[i])
            {
                tf_class player_class = (tf_class) g_pLocalPlayer->clazz;
                // This is pretty much always true, however, there are exceptions.
                int slot = i;
                // Non spy can't change PDA (and engineer doesn't need to)
                if (i == 3 && player_class != tf_spy)
                {
                    equip_stock[i] = false;
                    continue;
                }
                // For spy the primary is in reality a secondary. No idea why they did that
                if (player_class == tf_spy && slot == 0)
                    slot = 1;
                // PDA slot ids are way different, adjust
                if (slot == 3)
                    slot = 6;

                // If the timer is running for longer than 10s this is the first call after update
                stock_equip_reset[i].test_and_set(10000);

                // After 3 seconds we should stop equipping
                if (stock_equip_reset[i].check(3000))
                    equip_stock[i] = false;

                // Equip the item (-1 for stock)
                equip_item(player_class, slot, -1);
            }
        }
    if (!hat_troll)
        return;

    if (hat_steal_timer.test_and_set(15000))
    {
        std::rotate(hat_list.begin(), hat_list.begin() + 1, hat_list.end());
        equip_hats_fn(hat_list, { g_pLocalPlayer->clazz, g_pLocalPlayer->clazz });
    }
}

void Callback(int after, int type)
{
    if (!after)
        return;
    std::vector<Autoequip_unlock_list> equip_from;
    // Equip stock weapons
    if (after == 100 && type <= 3)
        equip_stock[type] = true;
    // Store the needed array
    switch (type)
    {
    case 0:
        if (after > primary.size())
            return;
        equip_from = primary;
        // primary
        break;
    case 1:
        if (after > secondary.size())
            return;
        equip_from = secondary;
        // secondary
        break;
    case 2:
        if (after > melee.size())
            return;
        equip_from = melee;
        // melee
        break;
    case 3:
        if (after > pda2.size())
            return;
        equip_from = pda2;
        // PDA 2
        break;
    }
    if (equip_from.size())
    {
        // Needs to get passed std vector of ints
        std::vector<int> pass = { equip_from.at(after - 1).achievement_id };
        // Unlock achievements and start accepting
        unlock_achievements_and_accept(pass);
        // equip queue
        equip_queue.push_back(equip_from.at(after - 1));
        // Start equip process
        equip = true;
    }
}

void Paint()
{
    // Start accepting
    if (accept_notifs)
    {
        accept_time.update();
        accept_notifs = false;
    }
    // "Trigger/Accept first notification" aka Achievement items
    if (!accept_time.check(5000) && cooldowm.test_and_set(500))
        g_IEngine->ClientCmd_Unrestricted("cl_trigger_first_notification");

    // Noisemaker code
    if (auto_noisemaker && equip_noisemaker.test_and_set(60000))
    {
        // Inventory Manager
        auto invmng = re::CTFInventoryManager::GTFInventoryManager();
        // Inventory
        auto inv = invmng->GTFPlayerInventory();

        // No Noisemaker in inventory?
        if (!inv->GetFirstItemOfItemDef(673) && !inv->GetFirstItemOfItemDef(536))
            accept_time.update();
        else
        {
            bool success = equip_action_item(673, { tf_scout, tf_engineer });
            if (!success)
            {
                success = equip_action_item(536, { tf_scout, tf_engineer });
                if (!success)
                    logging::Info("Bruh moment while equipping noisemaker");
            }
        }
    }
    // Hat equip code
    if (equip_hats)
    {
        // If done start accepting notifications, also time out after a while
        if (accept_time.check(5000) && !accept_time.check(10000) && cooldowm.test_and_set(500))
        {
            // Inventory Manager
            auto invmng = re::CTFInventoryManager::GTFInventoryManager();
            // Inventory
            auto inv = invmng->GTFPlayerInventory();
            // Frontline field recorder
            auto item_view1 = inv->GetFirstItemOfItemDef(302);
            // Gibus
            auto item_view2 = inv->GetFirstItemOfItemDef(940);
            // Skull Island Tropper
            auto item_view3 = inv->GetFirstItemOfItemDef(941);
            if (item_view1 && item_view2 && item_view3)
            {
                if (!accept_time.check(7500))
                {
                    // Equip these hats on all classes
                    bool success = equip_hats_fn({ 302, 940, 941 }, { tf_scout, tf_engineer });
                    if (success)
                    {
                        logging::Info("Equipping hats!");
                        equip_hats = false;
                    }
                }
            }
        }
        else if (accept_time.check(10000))
            equip_hats = false;
    }
    // Equip weapons
    if (equip)
    {
        // After 5 seconds of accept time, start
        if (accept_time.check(5000) && !accept_time.check(10000) && cooldown_2.test_and_set(500))
        {
            // Watch for each item and equip it
            for (int i = 0; i < equip_queue.size(); i++)
            {
                auto equip   = equip_queue.at(i);
                bool success = equip_item(equip.player_class, equip.slot, equip.item_id);

                if (success)
                {
                    logging::Info("Equipped Item!");
                    equip_queue.erase(equip_queue.begin() + i);
                }
            }
            // We did it
            if (!equip_queue.size())
                equip = false;
        }
        else if (accept_time.check(10000) && cooldown_2.test_and_set(500))
        {
            if (equip_queue.size())
            {
                logging::Info("Equipping failed!");
                equip_queue.clear();
            }
        }
    }
};

static InitRoutine init([]() {
    // Primary list
    primary.push_back(Autoequip_unlock_list("Force-A-Nature", 1036, 45, tf_scout, 0));
    primary.push_back(Autoequip_unlock_list("Backburner", 1638, 40, tf_pyro, 0));
    primary.push_back(Autoequip_unlock_list("Natascha", 1538, 41, tf_heavy, 0));
    primary.push_back(Autoequip_unlock_list("Frontier Justice", 1801, 141, tf_engineer, 0));
    primary.push_back(Autoequip_unlock_list("Blutsauger", 1437, 36, tf_medic, 0));
    primary.push_back(Autoequip_unlock_list("Huntsman", 1136, 56, tf_sniper, 0));
    primary.push_back(Autoequip_unlock_list("Ambassador", 1735, 61, tf_spy, 1));
    primary.push_back(Autoequip_unlock_list("Direct hit", 1237, 127, tf_soldier, 0));

    // Secondary list
    secondary.push_back(Autoequip_unlock_list("Bonk!", 1038, 46, tf_scout, 1));
    secondary.push_back(Autoequip_unlock_list("Buff Banner", 1238, 129, tf_soldier, 1));
    secondary.push_back(Autoequip_unlock_list("Flare gun", 1637, 39, tf_pyro, 1));
    secondary.push_back(Autoequip_unlock_list("Chargin' Targe", 1336, 131, tf_demoman, 1));
    secondary.push_back(Autoequip_unlock_list("Scottish Resistance", 1638, 130, tf_demoman, 1));
    secondary.push_back(Autoequip_unlock_list("Sandvich", 1537, 42, tf_heavy, 1));
    secondary.push_back(Autoequip_unlock_list("Wrangler", 1803, 140, tf_engineer, 1));
    secondary.push_back(Autoequip_unlock_list("Kritzkrieg", 1438, 35, tf_medic, 1));
    secondary.push_back(Autoequip_unlock_list("Jarate", 1137, 58, tf_sniper, 1));
    secondary.push_back(Autoequip_unlock_list("Razorback", 1138, 57, tf_sniper, 1));

    // Melee list
    melee.push_back(Autoequip_unlock_list("Sandman", 1037, 44, tf_scout, 2));
    melee.push_back(Autoequip_unlock_list("Equalizer", 1236, 128, tf_soldier, 2));
    melee.push_back(Autoequip_unlock_list("Axtinguisher", 1639, 38, tf_pyro, 2));
    melee.push_back(Autoequip_unlock_list("Eyelander", 1337, 132, tf_demoman, 2));
    melee.push_back(Autoequip_unlock_list("Killing Gloves of Boxing", 1539, 43, tf_heavy, 2));
    melee.push_back(Autoequip_unlock_list("Gunslinger", 1802, 142, tf_engineer, 2));
    melee.push_back(Autoequip_unlock_list("Ubersaw", 1439, 37, tf_medic, 2));

    // PDA 2
    pda2.push_back(Autoequip_unlock_list("Cloak and dagger", 1736, 60, tf_spy, 6));
    pda2.push_back(Autoequip_unlock_list("Deadringer", 1737, 59, tf_spy, 6));

    // Callbacks
    equip_primary.installChangeCallback([](settings::VariableBase<int> &, int after) { Callback(after, 0); });
    equip_secondary.installChangeCallback([](settings::VariableBase<int> &, int after) { Callback(after, 1); });
    equip_melee.installChangeCallback([](settings::VariableBase<int> &, int after) { Callback(after, 2); });
    equip_pda2.installChangeCallback([](settings::VariableBase<int> &, int after) { Callback(after, 3); });

    EC::Register(EC::CreateMove, CreateMove, "cm_nohatsforyou");
    EC::Register(EC::Paint, Paint, "achievement_autounlock");
    hat_troll.installChangeCallback([](settings::VariableBase<bool> &, bool after) {
        static bool init = false;
        if (after && !init)
        {
            hacks::shared::misc::generate_schema();
            hacks::shared::misc::Schema_Reload();
            init = true;
        }
    });
});
} // namespace hacks::tf2::achievement
