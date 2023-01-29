/*
 * Spam.cpp
 *
 *  Created on: Jan 21, 2017
 *      Author: nullifiedcat
 */

#include <hacks/Spam.hpp>
#include <settings/Bool.hpp>
#include <settings/String.hpp>
#include "common.hpp"
#include "MiscTemporary.hpp"

namespace hacks::shared::spam
{
static settings::Int spam_source{ "spam.source", "0" };
static settings::Boolean random_order{ "spam.random", "0" };
static settings::String filename{ "spam.filename", "spam.txt" };
static settings::Int spam_delay{ "spam.delay", "800" };
static settings::Int voicecommand_spam{ "spam.voicecommand", "0" };
static settings::Boolean teamname_spam{ "spam.teamname", "0" };
static settings::String teamname_file{ "spam.teamname.file", "teamspam.txt" };
static settings::Boolean team_only{ "spam.teamchat", "false" };

static size_t last_index;

std::chrono::time_point<std::chrono::system_clock> last_spam_point{};

static size_t current_index{ 0 };
static TextFile file{};

const std::string teams[] = { "RED", "BLU" };

// FUCK enum class.
// It doesn't have bitwise operators by default!! WTF!! static_cast<int>(REEE)!

enum class QueryFlags
{
    ZERO        = 0,
    TEAMMATES   = (1 << 0),
    ENEMIES     = (1 << 1),
    STATIC      = (1 << 2),
    ALIVE       = (1 << 3),
    DEAD        = (1 << 4),
    LOCALPLAYER = (1 << 5)
};

enum class QueryFlagsClass
{
    SCOUT    = (1 << 0),
    SNIPER   = (1 << 1),
    SOLDIER  = (1 << 2),
    DEMOMAN  = (1 << 3),
    MEDIC    = (1 << 4),
    HEAVY    = (1 << 5),
    PYRO     = (1 << 6),
    SPY      = (1 << 7),
    ENGINEER = (1 << 8)
};

struct Query
{
    int flags{ 0 };
    int flags_class{ 0 };
};

static int current_static_index{ 0 };
static Query static_query{};

bool PlayerPassesQuery(Query query, int idx)
{
    player_info_s pinfo;
    if (idx == g_IEngine->GetLocalPlayer())
    {
        if (!(query.flags & static_cast<int>(QueryFlags::LOCALPLAYER)))
            return false;
    }
    if (!GetPlayerInfo(idx, &pinfo))
        return false;
    CachedEntity *player = ENTITY(idx);
    if (!RAW_ENT(player))
        return false;
    int teammate = !player->m_bEnemy();
    bool alive   = !CE_BYTE(player, netvar.iLifeState);
    int clazzBit = (1 << (CE_INT(player, netvar.iClass) - 1));
    if (static_cast<int>(query.flags_class))
    {
        if (!(clazzBit & static_cast<int>(query.flags_class)))
            return false;
    }
    if (query.flags & (static_cast<int>(QueryFlags::TEAMMATES) | static_cast<int>(QueryFlags::ENEMIES)))
    {
        if (!teammate && !(query.flags & static_cast<int>(QueryFlags::ENEMIES)))
            return false;
        if (teammate && !(query.flags & static_cast<int>(QueryFlags::TEAMMATES)))
            return false;
    }
    if (query.flags & (static_cast<int>(QueryFlags::ALIVE) | static_cast<int>(QueryFlags::DEAD)))
    {
        if (!alive && !(query.flags & static_cast<int>(QueryFlags::DEAD)))
            return false;
        if (alive && !(query.flags & static_cast<int>(QueryFlags::ALIVE)))
            return false;
    }
    return true;
}

Query QueryFromSubstring(const std::string &string)
{
    Query result{};
    bool read = true;
    for (auto it = string.begin(); read && *it; it++)
    {
        if (*it == '%')
            read = false;
        if (read)
        {
            switch (*it)
            {
            case 's':
                result.flags |= static_cast<int>(QueryFlags::STATIC);
                break;
            case 'a':
                result.flags |= static_cast<int>(QueryFlags::ALIVE);
                break;
            case 'd':
                result.flags |= static_cast<int>(QueryFlags::DEAD);
                break;
            case 't':
                result.flags |= static_cast<int>(QueryFlags::TEAMMATES);
                break;
            case 'e':
                result.flags |= static_cast<int>(QueryFlags::ENEMIES);
                break;
            case 'l':
                result.flags |= static_cast<int>(QueryFlags::LOCALPLAYER);
                break;
            case '1':
                result.flags_class |= static_cast<int>(QueryFlagsClass::SCOUT);
                break;
            case '2':
                result.flags_class |= static_cast<int>(QueryFlagsClass::SOLDIER);
                break;
            case '3':
                result.flags_class |= static_cast<int>(QueryFlagsClass::PYRO);
                break;
            case '4':
                result.flags_class |= static_cast<int>(QueryFlagsClass::DEMOMAN);
                break;
            case '5':
                result.flags_class |= static_cast<int>(QueryFlagsClass::HEAVY);
                break;
            case '6':
                result.flags_class |= static_cast<int>(QueryFlagsClass::ENGINEER);
                break;
            case '7':
                result.flags_class |= static_cast<int>(QueryFlagsClass::MEDIC);
                break;
            case '8':
                result.flags_class |= static_cast<int>(QueryFlagsClass::SNIPER);
                break;
            case '9':
                result.flags_class |= static_cast<int>(QueryFlagsClass::SPY);
                break;
            }
        }
    }
    return result;
}

int QueryPlayer(Query query)
{
    if (query.flags & static_cast<int>(QueryFlags::STATIC))
    {
        if (current_static_index && (query.flags & static_query.flags) == static_query.flags && (query.flags_class & static_query.flags_class) == static_query.flags_class)
        {
            if (PlayerPassesQuery(query, current_static_index))
            {
                return current_static_index;
            }
        }
    }
    std::vector<int> candidates{};
    int index_result = 0;
    for (auto const &ent: entity_cache::player_cache)
    {
        int i = ent->m_IDX;
        if (PlayerPassesQuery(query, i))
        {
            candidates.push_back(i);
        }
    }
    if (candidates.size())
    {
        index_result = candidates.at(rand() % candidates.size());
    }
    if (query.flags & static_cast<int>(QueryFlags::STATIC))
    {
        current_static_index     = index_result;
        static_query.flags       = query.flags;
        static_query.flags_class = query.flags_class;
    }
    return index_result;
}

bool SubstituteQueries(std::string &input)
{
    size_t index = input.find("%query:");
    while (index != std::string::npos)
    {
        std::string sub = input.substr(index + 7);
        size_t closing  = sub.find("%");
        Query q         = QueryFromSubstring(sub);
        int p           = QueryPlayer(q);
        if (!p)
            return false;
        player_info_s pinfo;
        if (!GetPlayerInfo(p, &pinfo))
            return false;
        std::string name = std::string(pinfo.name);
        input.replace(index, 8 + closing, name);
        index = input.find("%query:", index + name.size());
    }
    return true;
}

bool FormatSpamMessage(std::string &message)
{
    ReplaceSpecials(message);
    bool team       = g_pLocalPlayer->team - 2;
    bool enemy_team = !team;
    IF_GAME(IsTF2())
    {
        ReplaceString(message, "%myteam%", teams[team]);
        ReplaceString(message, "%enemyteam%", teams[enemy_team]);
    }
    return SubstituteQueries(message);
}

// What to spam
static std::vector<std::string> teamspam_text = { "CAT", "HOOK" };
// Current spam index
static size_t current_teamspam_idx = 0;

static void CreateMove()
{
    IF_GAME(IsTF2())
    {
        // Spam changes the tournament name in casual and compeditive gamemodes
        if (teamname_spam)
        {
            if (!(g_GlobalVars->tickcount % 10))
            {
                if (teamspam_text.size())
                {
                    // We've hit the end of the vector, loop back to the front
                    // We need to do it like this, otherwise a file reload happening could cause this to crash at ".at"
                    if (current_teamspam_idx >= teamspam_text.size())
                        current_teamspam_idx = 0;
                    g_IEngine->ServerCmd(format("tournament_teamname ", teamspam_text.at(current_teamspam_idx)).c_str());
                    current_teamspam_idx++;
                }
            }
        }

        if (voicecommand_spam)
        {
            static Timer last_voice_spam;
            if (last_voice_spam.test_and_set(4000))
            {
                switch (*voicecommand_spam)
                {
                case 1: // RANDOM
                    g_IEngine->ServerCmd(format("voicemenu ", UniformRandomInt(0, 2), " ", UniformRandomInt(0, 8)).c_str());
                    break;
                case 2: // MEDIC
                    g_IEngine->ServerCmd("voicemenu 0 0");
                    break;
                case 3: // THANKS
                    g_IEngine->ServerCmd("voicemenu 0 1");
                    break;
                case 4: // NICE SHOT
                    g_IEngine->ServerCmd("voicemenu 2 6");
                    break;
                case 5: // CHEERS
                    g_IEngine->ServerCmd("voicemenu 2 2");
                    break;
                case 6: // JEERS
                    g_IEngine->ServerCmd("voicemenu 2 3");
                }
            }
        }
    }

    if (!spam_source)
        return;
    static int safety_ticks = 0;
    static int last_source  = 0;
    if (*spam_source != last_source)
    {
        safety_ticks = 300;
        last_source  = *spam_source;
    }
    if (safety_ticks > 0)
    {
        safety_ticks--;
        return;
    }
    else
    {
        safety_ticks = 0;
    }

    const std::vector<std::string> *source = nullptr;
    switch (*spam_source)
    {
    case 1:
        source = &file.lines;
        break;
    case 2:
        source = &builtin_default;
        break;
    case 3:
        source = &builtin_lennyfaces;
        break;
    case 4:
        source = &builtin_blanks;
        break;
    case 5:
        source = &builtin_nonecore;
        break;
    case 6:
        source = &builtin_lmaobox;
        break;
    case 7:
        source = &builtin_lithium;
        break;
    default:
        return;
    }
    if (!source || !source->size())
        return;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_spam_point).count() > int(spam_delay))
    {
        if (chat_stack::stack.empty())
        {
            if (current_index >= source->size())
                current_index = 0;
            if (random_order && source->size() > 1)
            {
                current_index = UniformRandomInt(0, source->size() - 1);
                while (current_index == last_index)
                {
                    current_index = UniformRandomInt(0, source->size() - 1);
                }
            }
            last_index             = current_index;
            std::string spamString = source->at(current_index);
            if (FormatSpamMessage(spamString))
                chat_stack::Say(spamString, *team_only);
            current_index++;
        }
        last_spam_point = std::chrono::system_clock::now();
    }
}

void reloadSpamFile()
{
    file.Load(*filename);
}

bool isActive()
{
    return bool(spam_source);
}

void init()
{
    spam_source.installChangeCallback([](settings::VariableBase<int> &var, int after) { file.Load(*filename); });
    filename.installChangeCallback([](settings::VariableBase<std::string> &var, std::string after) { file.TryLoad(after); });
    reloadSpamFile();
}

const std::vector<std::string> builtin_default    = { "Cathook - more fun than a ball of yarn!", "GNU/Linux is the best OS!", "Visit https://cathook.club for more information!", "Cathook - Free and Open-Source tf2 cheat!", "Cathook - ca(n)t stop me meow!" };
const std::vector<std::string> builtin_lennyfaces = { "( ͡° ͜ʖ ͡°)", "( ͡°( ͡° ͜ʖ( ͡° ͜ʖ ͡°)ʖ ͡°) ͡°)", "ʕ•ᴥ•ʔ", "(▀̿Ĺ̯▀̿ ̿)", "( ͡°╭͜ʖ╮͡° )", "(ง'̀-'́)ง", "(◕‿◕✿)", "༼ つ  ͡° ͜ʖ ͡° ༽つ" };
const std::vector<std::string> builtin_blanks     = { "\e"
                                                  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                                                  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                                                  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                                                  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                                                  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                                                  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                                                  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" };

const std::vector<std::string> builtin_nonecore = { "NULL CORE - REDUCE YOUR RISK OF BEING OWNED!", "NULL CORE - WAY TO THE TOP!", "NULL CORE - BEST TF2 CHEAT!", "NULL CORE - NOW WITH BLACKJACK AND HOOKERS!", "NULL CORE - BUTTHURT IN 10 SECONDS FLAT!", "NULL CORE - WHOLE SERVER OBSERVING!", "NULL CORE - GET BACK TO PWNING!", "NULL CORE - WHEN PVP IS TOO HARDCORE!", "NULL CORE - CAN CAUSE KIDS TO RAGE!", "NULL CORE - F2P NOOBS WILL BE 100% NERFED!" };
const std::vector<std::string> builtin_lmaobox  = { "GET GOOD, GET LMAOBOX!", "LMAOBOX - WAY TO THE TOP", "WWW.LMAOBOX.NET - BEST FREE TF2 HACK!" };
const std::vector<std::string> builtin_lithium  = { "CHECK OUT www.YouTube.com/c/DurRud FOR MORE INFORMATION!", "PWNING AIMBOTS WITH OP ANTI-AIMS SINCE 2015 - LITHIUMCHEAT", "STOP GETTING MAD AND STABILIZE YOUR MOOD WITH LITHIUMCHEAT!", "SAVE YOUR MONEY AND GET LITHIUMCHEAT! IT IS FREE!", "GOT ROLLED BY LITHIUM? HEY, THAT MEANS IT'S TIME TO GET LITHIUMCHEAT!!" };

void teamspam_reload(std::string after)
{
    // Clear spam vector
    teamspam_text.clear();
    // Reset Spam idx
    current_teamspam_idx = 0;
    if (after != "")
    {
        static TextFile teamspam;
        if (teamspam.TryLoad(after))
        {
            teamspam_text = teamspam.lines;
            for (auto &text : teamspam_text)
                ReplaceSpecials(text);
        }
    }
}
void teamspam_reload_command()
{
    teamspam_reload(*teamname_file);
}
static InitRoutine EC([]() {
    teamname_file.installChangeCallback([](settings::VariableBase<std::string> &, std::string after) { teamspam_reload(after); });
    EC::Register(EC::CreateMove, CreateMove, "spam", EC::average);
    init();
});

static CatCommand reload_ts("teamspam_reload", "Relaod teamspam file", teamspam_reload_command);

static CatCommand reload_cc("spam_reload", "Reload spam file", hacks::shared::spam::reloadSpamFile);
} // namespace hacks::shared::spam
