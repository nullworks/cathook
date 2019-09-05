#include "config.h"
#if ENABLE_IRC
#include "common.hpp"
#include "irc.hpp"
#include "CatBot.hpp"
#include "ChIRC.hpp"
#include "hack.hpp"
#include "ucccccp.hpp"
#include "PlayerTools.hpp"

namespace IRC
{
static settings::Boolean enabled("irc.enabled", "true");
static settings::Boolean anon("irc.anon", "true");
static settings::Boolean authenticate("irc.auth", "true");
static settings::String channel("irc.channel", "#cat_comms");
static settings::String address("irc.address", "irc.cathook.club");
static settings::Int port("irc.port", "8080");
static settings::String commandandcontrol_channel("irc.cc.channel", "");
static settings::String commandandcontrol_password("irc.cc.password", "");

static settings::Boolean transfer_leader_on_kick("irc.cc.leader-transfer", "false");
static settings::Boolean botonly("irc.cc.command-bot-only", "true");
static settings::Boolean irc_party{ "irc.cc.party", "false" };
static settings::Boolean answer_steam{ "irc.cc.respondparty", "false" };
static settings::Int party_cooldown{ "irc.cc.party-cooldown", "60" };
static settings::Int party_size{ "irc.cc.party-size", "6" };
static Timer last_sent_steamid{};
static Timer last_steamid_received{};
static std::vector<unsigned> steamidvec{};

static ChIRC::ChIRC irc;

void printmsg(std::string &usr, std::string &msg)
{
#if !ENFORCE_STREAM_SAFETY
    if (msg.size() > 256 || usr.size() > 256)
    {
        logging::Info("IRC: Message too large.");
        return;
    }
    if (g_Settings.bInvalid)
        logging::Info("[IRC] %s: %s", usr.c_str(), msg.c_str());
    else
        PrintChat("\x07%06X[IRC] %s\x01: %s", 0xe05938, usr.c_str(), msg.c_str());
#endif
}
void printmsgcopy(std::string usr, std::string msg)
{
    printmsg(usr, msg);
}

namespace handlers
{
void message(std::string &usr, std::string &msg)
{
    std::string toprint = msg.substr(3);
    if (toprint.empty())
        return;
    printmsg(usr, toprint);
}
void authreq(std::string &msg)
{
    // Check if we are in a game
    if (g_Settings.bInvalid)
        return;
    bool isreply = false;

    if (msg.find("authrep") == 0)
        isreply = true;

    // Get steamid hash from string
    std::string steamidhash;
    if (isreply)
        steamidhash = msg.substr(7);
    else
        steamidhash = msg.substr(4);

    for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
    {
        if (i == g_pLocalPlayer->entity_idx)
            continue;
        player_info_s pinfo;
        // Get playerinfo and check if player on server
        if (!g_IEngine->GetPlayerInfo(i, &pinfo))
            continue;
        auto tarsteamid        = pinfo.friendsID;
        std::string total_hash = std::to_string(tarsteamid) + pinfo.name;
        MD5Value_t result;
        // Hash steamid
        MD5_ProcessSingleBuffer(total_hash.c_str(), strlen(total_hash.c_str()), result);
        // Get bits of hash and store in string
        std::string tarhash;
        for (auto i : result.bits)
        {
            for (int j = 0; j < 8; j++)
                tarhash.append(std::to_string((i >> j) & 1));
        }
        // Check if steamid of sender == steamid we currently check
        // (using hashes)
        if (tarhash == steamidhash)
        {
            // Use actual steamid to set cat status
            playerlist::ChangeState(tarsteamid, playerlist::k_EState::CAT);
            // Avoid replying to a reply
            if (isreply)
                // We are done here. Steamid duplicates don't exist.
                return;
            // If message is not a reply, reply.
            auth(true);
            // We are done here. Steamid duplicates don't exist.
            return;
        }
    }
}

void cc_party(std::string &msg)
{
    auto party_client = re::CTFPartyClient::GTFPartyClient();
    if (!party_client)
        return;
    int online_members = party_client->GetNumOnlineMembers();
    int members        = party_client->GetNumMembers();
    if (msg.find("cc_partysteamrep") == 0 && ((online_members < *party_size && online_members != 6) || online_members < members))
    {
        if (!irc_party)
            return;
        unsigned steamid;
        try
        {
            steamid = std::stoul(msg.substr(16));
        }
        catch (std::invalid_argument)
        {
            return;
        }
        if (std::find(steamidvec.begin(), steamidvec.end(), steamid) == steamidvec.end())
        {
            steamidvec.push_back(steamid);
            last_steamid_received.update();
        }
    }
    else if (answer_steam && msg.find("cc_partysteam") == 0 && ((online_members < *party_size && online_members != 6) || online_members < members))
    {
        irc.privmsg(format("cc_partysteamrep", g_ISteamUser->GetSteamID().GetAccountID()), true);
        unsigned steamid;
        try
        {
            steamid = std::stoul(msg.substr(13));
        }
        catch (std::invalid_argument)
        {
            return;
        }
        if (std::find(steamidvec.begin(), steamidvec.end(), steamid) == steamidvec.end())
            steamidvec.push_back(steamid);
    }
}
void cc_cmd(std::string &msg)
{
    if (!hacks::shared::catbot::catbotmode && botonly)
        return;
    // Outdated cc_cmd. Return
    if (msg.find("$cmd") == msg.npos)
        return;
    // Command applys to all bots
    if (msg.find("$id") == msg.npos)
    {
        // Todo: Remove debug
        std::cout << msg.substr(msg.find("$cmd") + 4) << std::endl;
        hack::ExecuteCommand(msg.substr(msg.find("$cmd") + 4));
    }
    else
    {
        std::string string_id = msg.substr(msg.find("$id") + 3, msg.find("$cmd") - (msg.find("$id") + 3));
        int id;
        // Todo: Remove debug
        std::cout << "id:" << string_id << std::endl;
        try
        {
            id = std::stoi(string_id);
        }
        catch (std::invalid_argument)
        {
            // id is not int???
            return;
        }
        if (id == irc.getData().id)
        {
            // Thats me!
            std::cout << msg.substr(msg.find("$cmd") + 4) << std::endl;
            hack::ExecuteCommand(msg.substr(msg.find("$cmd") + 4));
        }
    }
}
} // namespace handlers

void handleIRC(IRCMessage message, IRCClient *client)
{
    std::string &cmd = message.command;
    try
    {
        message.parameters.at(0);
        message.parameters.at(1);
    }
    catch (std::out_of_range)
    {
        logging::Info("Something is out of range");
        return;
    }
    std::string &channel = message.parameters.at(0);
    std::string &rawmsg  = message.parameters.at(1);
    std::string &usr     = message.prefix.nick;
    if (!ucccccp::validate(rawmsg))
        return;
    std::string msg(ucccccp::decrypt(rawmsg));
    if (msg == "Attempt at ucccccping and failing" || msg == "Unsupported version")
        return;

    // Handle privmsg (Message to #channel)
    if (cmd == "PRIVMSG")
    {
        if (msg.empty() || usr.empty())
            return;
        // Handle public messages
        if (channel == irc.getData().comms_channel)
        {
            // Handle messages
            if (msg.find("msg") == 0)
            {
                handlers::message(usr, msg);
                return;
            }
            // Handle auth requests
            else if (msg.find("auth") == 0)
            {
                handlers::authreq(msg);
            }
        }
        else if (channel == irc.getData().commandandcontrol_channel)
        {
            if (msg.find("cc_cmd") == 0)
            {
                handlers::cc_cmd(msg);
            }
            if (msg.find("cc_party") == 0)
            {
                handlers::cc_party(msg);
            }
        }
    }
}

void updateData()
{
    std::string nick("Anon");
    if (!*anon)
        nick = g_ISteamFriends->GetPersonaName();
    irc.UpdateData(nick, nick, *channel, *commandandcontrol_channel, *commandandcontrol_password, *address, *port, *hacks::shared::catbot::catbotmode, g_ISteamUser->GetSteamID().GetAccountID());
}

bool sendmsg(std::string &msg, bool loopback)
{
    std::string raw = "msg" + msg;
    if (irc.privmsg(raw))
    {
        if (loopback)
        {
            printmsgcopy(irc.getData().nick, msg);
        }
        return true;
    }
    if (loopback)
        printmsgcopy("Cathook", "Error! Couldn't send message.");
    return false;
}

void auth(bool reply)
{
    if (g_Settings.bInvalid && !g_Settings.is_create_move)
        return;
    if (!*authenticate)
        return;
    MD5Value_t result;
    player_info_s pinfo{};
    if (!g_IEngine->GetPlayerInfo(LOCAL_E->m_IDX, &pinfo))
        return;
    std::string total_hash = std::to_string(pinfo.friendsID) + pinfo.name;
    MD5_ProcessSingleBuffer(total_hash.c_str(), strlen(total_hash.c_str()), result);
    std::string msg("auth");
    if (reply)
        msg.append("rep");
    for (auto i : result.bits)
    {
        for (int j = 0; j < 8; j++)
            msg.append(std::to_string((i >> j) & 1));
    }
    irc.privmsg(msg);
}

static bool restarting{ false };

Timer calledonce{};
Timer ircstate{};
int GetMaxParty()
{
    int partyable = 1;
    auto peers    = irc.getPeers();
    for (auto peer : peers)
    {
        if (peer.second.party_size != -1)
            partyable++;
    }
    return partyable;
}
CatCommand debug_maxparty("debug_partysize", "Debug party size", []() { logging::Info("%d", GetMaxParty()); });
CatCommand debug_steamids("debug_steamids", "Debug steamids", []() {
    for (auto &i : irc.getPeers())
        logging::Info("%u", i.second.steamid);
});
static Timer resize_party{};
static Timer pass_leader{};

void party_leader_pass()
{
    if (pass_leader.test_and_set(10000))
    {
        re::CTFGCClientSystem *gc = re::CTFGCClientSystem::GTFGCClientSystem();
        re::CTFPartyClient *pc    = re::CTFPartyClient::GTFPartyClient();
        if (gc && gc->BHaveLiveMatch() && pc->GetNumMembers() > 1)
        {
            CSteamID steamid;
            pc->GetCurrentPartyLeader(steamid);
            if (steamid.GetAccountID() == g_ISteamUser->GetSteamID().GetAccountID())
            {
                std::vector<unsigned int> valid_steam_ids = pc->GetPartySteamIDs();
                bool found                                = false;
                for (auto &peer : irc.getPeers())
                {
                    if (found)
                        break;
                    if (!peer.second.is_ingame)
                    {
                        for (auto &id : valid_steam_ids)
                            if (id == peer.second.steamid)
                            {
                                CSteamID steam(id, EUniverse::k_EUniversePublic, EAccountType::k_EAccountTypeIndividual);
                                pc->PromotePlayerToLeader(steam);
                                found = true;
                                break;
                            }
                    }
                }
            }
        }
    }
}

static void run()
{
    if (!restarting)
    {
        auto party_client  = re::CTFPartyClient::GTFPartyClient();
        int online_members = party_client->GetNumOnlineMembers();
        int members        = party_client->GetNumMembers();

        if (irc_party && resize_party.test_and_set(10000) && party_client && (online_members > *party_size || online_members < members))
        {
            int lowest_id = INT_MAX;
            for (auto peer : irc.getPeers())
            {
                int id         = peer.first;
                int party_size = peer.second.party_size;
                if (party_size != online_members)
                    continue;
                if (id < lowest_id)
                    lowest_id = id;
            }
            if (irc.getData().id < lowest_id)
                hack::command_stack().push("tf_party_leave");
        }
        if (last_sent_steamid.check(8000) && calledonce.test_and_set(2000) && online_members < *party_size)
        {
            if (irc_party && !steamidvec.empty() && party_client && ((online_members != 6 && online_members < GetMaxParty()) || online_members != members))
            {
                steamidvec.push_back(g_ISteamUser->GetSteamID().GetAccountID());
                int idx         = -1;
                unsigned lowest = UINT_MAX;
                for (int i = 0; i < steamidvec.size(); i++)
                    if (steamidvec[i] < lowest)
                    {
                        lowest = steamidvec[i];
                        idx    = i;
                    }
                if (idx != -1 && steamidvec[idx] != g_ISteamUser->GetSteamID().GetAccountID())
                    hack::command_stack().push("tf_party_leave");
            }
        }
        if (last_steamid_received.test_and_set(10000) && (online_members < *party_size || online_members < members))
        {
            if (party_client && ((online_members != 6 && online_members < GetMaxParty()) || online_members < members))
                if (!steamidvec.empty())
                {
                    steamidvec.push_back(g_ISteamUser->GetSteamID().GetAccountID());
                    int idx         = -1;
                    unsigned lowest = UINT_MAX;
                    for (int i = 0; i < steamidvec.size(); i++)
                        if (steamidvec[i] < lowest)
                        {
                            lowest = steamidvec[i];
                            idx    = i;
                        }
                    if (idx != -1 && steamidvec[idx] != g_ISteamUser->GetSteamID().GetAccountID())
                    {
                        hack::command_stack().push(format("tf_party_request_join_user ", steamidvec[idx]));
                    }
                    steamidvec.clear();
                }
        }
        if (irc_party && last_sent_steamid.test_and_set(*party_cooldown * 1000) && ((online_members < *party_size && online_members != 6) || online_members < members))
            irc.privmsg(format("cc_partysteam", g_ISteamUser->GetSteamID().GetAccountID()), true);
        irc.Update();
        if (ircstate.test_and_set(20000))
        {
            ChIRC::GameState state;
            int size;
            if (irc_party && answer_steam)
                size = online_members;
            else
                size = -1;
            state.party_size          = size;
            re::CTFGCClientSystem *gc = re::CTFGCClientSystem::GTFGCClientSystem();
            state.is_ingame           = gc && gc->BHaveLiveMatch();
            irc.setState(state);
        }
        if (transfer_leader_on_kick)
            party_leader_pass();
    }
}

template <typename T> void rvarCallback(settings::VariableBase<T> &var, T after)
{
    if (!restarting)
    {
        restarting = true;
        std::thread reload([]() {
            std::this_thread::sleep_for(std::chrono_literals::operator""ms(500));
            irc.Disconnect();
            updateData();
            if (enabled)
                irc.Connect();
            restarting = false;
        });
        reload.detach();
    }
}

static InitRoutine init([]() {
    EC::Register(EC::Paint, run, "PAINT_irc", EC::average);
    updateData();
    enabled.installChangeCallback(rvarCallback<bool>);
    anon.installChangeCallback(rvarCallback<bool>);
    authenticate.installChangeCallback(rvarCallback<bool>);
    channel.installChangeCallback(rvarCallback<std::string>);
    address.installChangeCallback(rvarCallback<std::string>);
    port.installChangeCallback(rvarCallback<int>);
    commandandcontrol_channel.installChangeCallback(rvarCallback<std::string>);
    commandandcontrol_password.installChangeCallback(rvarCallback<std::string>);

    irc.installCallback("PRIVMSG", handleIRC);
    if (enabled)
        irc.Connect();
});

static CatCommand irc_send_cmd("irc_send_cmd", "Send cmd to IRC", [](const CCommand &args) { irc.sendraw(args.ArgS()); });
static CatCommand irc_exec_all("irc_exec_all", "Send command to C&C channel", [](const CCommand &args) {
    std::string msg("cc_cmd$cmd");
    msg.append(args.ArgS());
    irc.privmsg(msg, true);
});

static CatCommand invite_all("irc_invite_all", "Inivte all people in C&C channel", [](const CCommand &args) {
    std::string msg("cc_cmd$cmdtf_party_request_join_user ");
    msg.append(std::to_string(g_ISteamUser->GetSteamID().GetAccountID()));
    irc.privmsg(msg, true);
});

static CatCommand irc_send("irc_send", "Send message to IRC", [](const CCommand &args) {
    std::string msg(args.ArgS());
    sendmsg(msg, true);
});

static CatCommand irc_auth("irc_auth", "Auth via IRC (Find users on same server)", []() { auth(); });

} // namespace IRC
#endif
