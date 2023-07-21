#include "config.h"
#if ENABLE_NULLNEXUS
#include "libnullnexus/nullnexus.hpp"
#include <boost/algorithm/string.hpp>
#include "DetourHook.hpp"
#include "nullnexus.hpp"
#if ENABLE_VISUALS
#include "colors.hpp"
#include "MiscTemporary.hpp"
#endif

namespace nullnexus
{
static settings::Boolean enabled("nullnexus.enabled", "true");
static settings::Boolean anon("nullnexus.user.anon", "false");
static settings::String address("nullnexus.host", "nullnexus.cathook.club");
static settings::String port("nullnexus.port", "3000");
static settings::String endpoint("nullnexus.endpoint", "/api/v1/client");
#if ENABLE_TEXTMODE
static settings::Boolean proxyenabled("nullnexus.proxy.enabled", "true");
#else
static settings::Boolean proxyenabled("nullnexus.proxy.enabled", "false");
#endif
static settings::String proxysocket("nullnexus.relay.socket", "/tmp/nullnexus.sock");
static settings::Boolean authenticate("nullnexus.auth", "true");
#if ENABLE_VISUALS
static settings::Rgba colour("nullnexus.user.colour");
#endif

static NullNexus nexus;

void printmsg(std::string &usr, std::string &msg, int colour = 0xff9340)
{
#if !ENFORCE_STREAM_SAFETY && ENABLE_VISUALS
    if (msg.size() > 128 || usr.size() > 32)
    {
        logging::Info("Nullnexus: Message too large.");
        return;
    }
    if (g_Settings.bInvalid)
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "[Nullnexus] %s: %s\n", usr.c_str(), msg.c_str());
    else
        PrintChat("\x07%06X[\x07%06XNullnexus\x07%06X] \x07%06X%s\x01: %s", 0x5e3252, 0xba3d9a, 0x5e3252, colour, usr.c_str(), msg.c_str());
#endif
}
void printmsgcopy(std::string usr, std::string msg)
{
    printmsg(usr, msg);
}

namespace handlers
{
void message(std::string usr, std::string msg, int colour)
{
    printmsg(usr, msg, colour);
}
void authedplayers(std::vector<std::string> steamids)
{
    // Check if we are in a game
    if (g_Settings.bInvalid)
        return;
    for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
    {
        player_info_s pinfo{};
        if (GetPlayerInfo(i, &pinfo))
        {
            if (pinfo.friendsID == 0)
                continue;
            MD5Value_t result;
            std::string steamidhash = std::to_string(pinfo.friendsID) + pinfo.name;
            MD5_ProcessSingleBuffer(steamidhash.c_str(), strlen(steamidhash.c_str()), result);
            std::stringstream ss;
            ss << std::hex;
            for (auto i : result.bits)
                ss << std::setw(2) << std::setfill('0') << (int) i;
            steamidhash = ss.str();
            std::remove_if(steamids.begin(), steamids.end(),
                           [&steamidhash, &pinfo](std::string &steamid)
                           {
                               if (steamid == steamidhash)
                               {
                                   // Use actual steamid to set cat status
                                   if (playerlist::ChangeState(pinfo.friendsID, playerlist::k_EState::CAT))
                                       PrintChat("\x07%06X%s\x01 Marked as CAT (Nullnexus user)", 0xe05938, pinfo.name);
                                   return true;
                               }
                               return false;
                           });
        }
    }
}
} // namespace handlers

static std::string server_steamid = "";

// Update info about the current server we are on.
void updateServer(NullNexus::UserSettings &settings)
{
    INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    // Additional currently inactive security measure, may be activated at any time
    static int *gHostSpawnCount = *reinterpret_cast<int **>(gSignatures.GetEngineSignature("A3 ? ? ? ? A1 ? ? ? ? 8B 10 89 04 24 FF 52 ? 83 C4 2C") + sizeof(char));
    if (ch && *authenticate && server_steamid != "")
    {
        // SDR Makes this unusable :(
        // auto addr = ch->GetRemoteAddress();

        player_info_s pinfo{};
        if (GetPlayerInfo(g_pLocalPlayer->entity_idx, &pinfo))
        {
            MD5Value_t result;
            std::string steamidhash = std::to_string(pinfo.friendsID) + pinfo.name;
            MD5_ProcessSingleBuffer(steamidhash.c_str(), strlen(steamidhash.c_str()), result);
            std::stringstream ss;
            ss << std::hex;
            for (auto i : result.bits)
                ss << std::setw(2) << std::setfill('0') << (int) i;
            steamidhash        = ss.str();
            settings.tf2server = NullNexus::TF2Server(true, server_steamid, steamidhash, *gHostSpawnCount);
            return;
        }
    }
    // Not connected
    settings.tf2server = NullNexus::TF2Server(false);
}

static bool waiting_status_data = false;

static DetourHook ProcessPrint_detour_hook;

typedef bool *(*ProcessPrint_t)(void *baseclient, SVC_Print *msg);

// Need to do this so the function below resolves
void updateServer();

bool ProcessPrint_detour_fn(void *baseclient, SVC_Print *msg)
{
    if (waiting_status_data && msg->m_szText)
    {
        auto msg_str = std::string(msg->m_szText);
        std::vector<std::string> lines;
        boost::split(lines, msg_str, boost::is_any_of("\n"), boost::token_compress_on);
        auto str = lines[0];

        if (str.rfind("steamid : ", 0) == 0)
        {
            waiting_status_data = false;

            if (str.length() < 12)
                server_steamid = "";
            else
            {
                str = str.substr(10);
                if (str == "not logged in" || str.rfind("]") == str.npos)
                    str = "";
                else
                    str = str.substr(0, str.rfind("]") + 1);
                server_steamid = str;
            }
            updateServer();
        }
    }

    auto original = (ProcessPrint_t) ProcessPrint_detour_hook.GetOriginalFunc();
    auto ret_val  = original(baseclient, msg);
    ProcessPrint_detour_hook.RestorePatch();
    return ret_val;
}

// Update server steamid
void updateSteamID()
{
    waiting_status_data = true;
    g_IEngine->ServerCmd("status");
}

// Update info about the current server we are on.
void updateServer()
{
    if (!g_IEngine->IsInGame() || server_steamid != "")
    {
        NullNexus::UserSettings settings;
        updateServer(settings);
        nexus.changeData(settings);
    }
    else
        updateSteamID();
}

void updateData()
{
    std::optional<std::string> username = std::nullopt;
    std::optional<int> newcolour        = std::nullopt;
    username                            = *anon ? "anon" : g_ISteamFriends->GetPersonaName();
#if ENABLE_VISUALS
    if ((*colour).r || (*colour).g || (*colour).b)
    {
        int r     = (*colour).r * 255;
        int g     = (*colour).g * 255;
        int b     = (*colour).b * 255;
        newcolour = (r << 16) + (g << 8) + b;
    }
#endif
    NullNexus::UserSettings settings;
    settings.username = *username;
    settings.colour   = newcolour;
    // Tell nullnexus about the current server we are connected to.
    updateServer(settings);

    nexus.changeData(settings);
}

bool sendmsg(std::string &msg)
{
    if (!enabled)
    {
        printmsgcopy("Cathook", "Error! Nullnexus is disabled!");
        return false;
    }
    if (nexus.sendChat(msg))
        return true;
    printmsgcopy("Cathook", "Error! Couldn't send message.");
    return false;
}

template <typename T> void rvarCallback(settings::VariableBase<T> &, T)
{
    std::thread reload(
        []()
        {
            std::this_thread::sleep_for(std::chrono_literals::operator""ms(500));
            updateData();
            if (*enabled)
            {
                if (*proxyenabled)
                    nexus.connectunix(*proxysocket, *endpoint, true);
                else
                    nexus.connect(*address, *port, *endpoint, true);
            }
            else
                nexus.disconnect();
        });
    reload.detach();
}

template <typename T> void rvarDataCallback(settings::VariableBase<T> &, T)
{
    std::thread reload(
        []()
        {
            std::this_thread::sleep_for(std::chrono_literals::operator""ms(500));
            updateData();
        });
    reload.detach();
}

static InitRoutine init(
    []()
    {
        updateData();
        enabled.installChangeCallback(rvarCallback<bool>);
        address.installChangeCallback(rvarCallback<std::string>);
        port.installChangeCallback(rvarCallback<std::string>);
        endpoint.installChangeCallback(rvarCallback<std::string>);

        proxyenabled.installChangeCallback(rvarCallback<bool>);
        proxysocket.installChangeCallback(rvarCallback<std::string>);

#if ENABLE_VISUALS
        colour.installChangeCallback(rvarDataCallback<rgba_t>);
#endif
        anon.installChangeCallback(rvarDataCallback<bool>);
        authenticate.installChangeCallback(rvarDataCallback<bool>);

        nexus.setHandlerChat(handlers::message);
        nexus.setHandlerAuthedplayers(handlers::authedplayers);
        if (*enabled)
        {
            if (*proxyenabled)
                nexus.connectunix(*proxysocket, *endpoint, true);
            else
                nexus.connect(*address, *port, *endpoint, true);
        }

        uintptr_t processprint_addr = gSignatures.GetEngineSignature("55 89 E5 56 53 83 EC 50 C7 45 ? 00 00 00 00 A1 ? ? ? ? C7 45 ? 00 00 00 00 85 C0 0F 84 ? ? ? ? 8D 55 ? 89 04 24 89 54 24 ? C7 44 24 ? ? ? ? ? C7 44 24 ? ? ? ? ? C7 44 24 ? ? ? ? ? C7 44 24 ? ? ? ? ? C7 44 24 ? 55 04 00 00");

        ProcessPrint_detour_hook.Init(processprint_addr, (void *) ProcessPrint_detour_fn);

        EC::Register(
            EC::Shutdown,
            []()
            {
                nexus.disconnect();
                ProcessPrint_detour_hook.Shutdown();
            },
            "shutdown_nullnexus");
        EC::Register(
            EC::FirstCM, []() { updateServer(); }, "firstcm_nullnexus");
        EC::Register(
            EC::LevelShutdown,
            []()
            {
                server_steamid = "";
                updateServer();
            },
            "firstcm_nullnexus");
    });
static CatCommand nullnexus_send("nullnexus_send", "Send message to IRC",
                                 [](const CCommand &args)
                                 {
                                     std::string msg(args.ArgS());
                                     sendmsg(msg);
                                 });
} // namespace nullnexus
#endif
