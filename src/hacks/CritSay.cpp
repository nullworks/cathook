/*
 * Created on 25.01.2019 By BenCat07
 * CritSay.cpp
 * Read if gay
 */
#include <settings/Int.hpp>
#include "common.hpp"

namespace hacks::shared::critsay
{
static settings::Int critsay_mode{ "critsay.mode", "0" };
static settings::String filename{ "critsay.file", "critsay.txt" };
static settings::Int delay{ "critsay.delay", "100" };

struct CritsayStorage
{
    Timer timer{};
    unsigned delay{};
    std::string message{};
};

static boost::unordered_flat_map<int, CritsayStorage> critsay_storage{};

// Thanks HellJustFroze for linking me http://daviseford.com/shittalk/
const std::vector<std::string> builtin_default = { "Woops, i slipped", "*critical hit* -> %name%", "ok now let's do it again, %name%", "nice" };

const std::string tf_classes_killsay[] = { "class", "scout", "sniper", "soldier", "demoman", "medic", "heavy", "pyro", "spy", "engineer" };
const std::string tf_teams_killsay[]   = { "RED", "BLU" };
static std::string lastmsg{};

TextFile file{};

std::string ComposeCritSay(IGameEvent *event)
{
    const std::vector<std::string> *source = nullptr;
    switch (*critsay_mode)
    {
    case 1:
        source = &file.lines;
        break;
    case 2:
        source = &builtin_default;
        break;
    default:
        break;
    }
    if (!source || source->empty())
        return "";
    if (!event)
        return "";
    if (!(event->GetInt("damagebits", 0) & (1 << 20)))
        return "";
    int vid = event->GetInt("userid");
    int kid = event->GetInt("attacker");
    if (kid == vid)
        return "";
    if (GetPlayerForUserID(kid) != g_IEngine->GetLocalPlayer())
        return "";
    //	checks if the killsays.txt file is not 1 line. 100% sure it's going
    // to crash if it is.
    std::string msg;
    do {
        msg = source->at(rand() % source->size());
    } while (msg == lastmsg && source->size() > 1);
    lastmsg = msg;
    player_info_s info{};
    GetPlayerInfo(GetPlayerForUserID(vid), &info);

    ReplaceSpecials(msg);
    CachedEntity *ent = ENTITY(GetPlayerForUserID(vid));
    int clz           = g_pPlayerResource->GetClass(ent);
    ReplaceString(msg, "%class%", tf_classes_killsay[clz]);
    player_info_s infok{};
    GetPlayerInfo(GetPlayerForUserID(kid), &infok);
    ReplaceString(msg, "%killer%", std::string(infok.name));
    ReplaceString(msg, "%team%", tf_teams_killsay[ent->m_iTeam() - 2]);
    ReplaceString(msg, "%myteam%", tf_teams_killsay[LOCAL_E->m_iTeam() - 2]);
    ReplaceString(msg, "%myclass%", tf_classes_killsay[g_pPlayerResource->GetClass(LOCAL_E)]);
    ReplaceString(msg, "%name%", std::string(info.name));
    return msg;
}

class CritSayEventListener : public IGameEventListener2
{
    void FireGameEvent(IGameEvent *event) override
    {
        if (!critsay_mode)
            return;
        std::string message = hacks::shared::critsay::ComposeCritSay(event);
        if (!message.empty())
        {
            int vid                    = event->GetInt("userid");
            critsay_storage[vid].delay = *delay;
            critsay_storage[vid].timer.update();
            critsay_storage[vid].message = message;
        }
    }
};

static void ProcessCritsay()
{
    if (critsay_storage.empty())
        return;
    for (auto &i : critsay_storage)
    {
        if (i.second.message.empty())
            continue;
        if (i.second.timer.test_and_set(i.second.delay))
        {
            chat_stack::Say(i.second.message, false);
            i.second = {};
        }
    }
}

static CritSayEventListener listener{};

void reload()
{
    file.Load(*filename);
}

static CatCommand reload_command("critsay_reload", "Reload critsays", []() { reload(); });

void init()
{
    g_IEventManager2->AddListener(&listener, (const char *) "player_death", false);
}

void shutdown()
{
    g_IEventManager2->RemoveListener(&listener);
}

static InitRoutine runinit(
    []()
    {
        EC::Register(EC::Paint, ProcessCritsay, "paint_critsay", EC::average);
        EC::Register(EC::Shutdown, shutdown, "shutdown_critsay", EC::average);
        critsay_mode.installChangeCallback(
            [](settings::VariableBase<int> &a, int value)
            {
                if (value == 1)
                    reload();
            });
        init();
    });

} // namespace hacks::shared::critsay
