/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <chatlog.hpp>
#include <boost/algorithm/string.hpp>
#include <MiscTemporary.hpp>
#include <hacks/AntiAim.hpp>
#include <settings/Bool.hpp>
#include "HookedMethods.hpp"
#include "CatBot.hpp"
#include "ChatCommands.hpp"
#include "MiscTemporary.hpp"
#include <iomanip>
#include "votelogger.hpp"

static settings::Boolean dispatch_log{ "debug.log-dispatch-user-msg", "false" };
static settings::Boolean chat_filter_enable{ "chat.censor.enable", "false" };
static settings::Boolean anti_votekick{ "cat-bot.anti-autobalance", "false" };

static bool retrun = false;
static Timer gitgud{};

// Using repeated char causes crash on some systems. Suboptimal solution.
const static std::string clear("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                               "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                               "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                               "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                               "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                               "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                               "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
std::string lastfilter{};
std::string lastname{};

namespace hacks::tf::autoheal
{
extern std::vector<int> called_medic;
}
namespace hooked_methods
{
static Timer sendmsg{};
template <typename T> void SplitName(std::vector<T> &ret, const T &name, int num)
{
    T tmp;
    int chars = 0;
    for (char i : name)
    {
        if (i == ' ')
            continue;

        tmp.push_back(std::tolower(i));
        ++chars;
        if (chars == num + 1)
        {
            chars = 0;
            ret.push_back(tmp);
            tmp.clear();
        }
    }
    if (tmp.size() > 2)
        ret.push_back(tmp);
}
static int anti_balance_attempts = 0;
static std::string previous_name = "";
static Timer reset_it{};
static Timer wait_timer{};
void Paint()
{
    if (!wait_timer.test_and_set(1000))
        return;
    INetChannel *server = (INetChannel *) g_IEngine->GetNetChannelInfo();
    if (server)
        reset_it.update();
    if (reset_it.test_and_set(20000))
    {
        anti_balance_attempts = 0;
        previous_name         = "";
    }
}
static InitRoutine Autobalance([]() { EC::Register(EC::Paint, Paint, "paint_autobalance", EC::average); });
DEFINE_HOOKED_METHOD(DispatchUserMessage, bool, void *this_, int type, bf_read &buf)
{
    if (!isHackActive())
        return original::DispatchUserMessage(this_, type, buf);

    int s, i, j;
    char c;
    const char *buf_data = reinterpret_cast<const char *>(buf.m_pData);

    /* Delayed print of name and message, censored by chat_filter
     * TO DO: Document type 47
     */
    if (retrun && type != 47 && gitgud.test_and_set(300))
    {
        PrintChat("\x07%06X%s\x01: %s", 0xe05938, lastname.c_str(), lastfilter.c_str());
        retrun = false;
    }
    std::string data;
    switch (type)
    {

    case 25:
    {
        // DATA = [ 01 01 06  ] For "Charge me Doctor!"
        // First Byte represents entityid
        int ent_id = buf.ReadByte();
        // Second Byte represents the voicemenu used for it
        int voice_menu = buf.ReadByte();
        // Third Byte represents the command in that voicemenu (starting from 0)
        int command_id = buf.ReadByte();

        if (voice_menu == 1 && command_id == 6)
            hacks::tf::autoheal::called_medic.push_back(ent_id);
        // If we don't .Seek(0) the game will have a bad  time reading
        buf.Seek(0);
        break;
    }
    case 12:
        if (hacks::shared::catbot::anti_motd && hacks::shared::catbot::catbotmode)
        {
            data = std::string(buf_data);
            if (data.find("class_") != data.npos)
                return false;
        }
        break;
    case 5:
    {
        if (*anti_votekick && buf.GetNumBytesLeft() > 35)
        {
            INetChannel *server = (INetChannel *) g_IEngine->GetNetChannelInfo();
            data                = std::string(buf_data);
            logging::Info("%s", data.c_str());
            if (data.find("TeamChangeP") != data.npos && CE_GOOD(LOCAL_E))
            {
                std::string server_name(server->GetAddress());
                if (server_name != previous_name)
                {
                    previous_name         = server_name;
                    anti_balance_attempts = 0;
                }
                if (anti_balance_attempts < 2)
                {
                    ignoredc = true;
                    g_IEngine->ClientCmd_Unrestricted("killserver;wait 100;cat_mm_join");
                }
                else
                {
                    std::string autobalance_msg = "tf_party_chat \"autobalanced in 3 seconds";
                    if (ipc::peer && ipc::peer->connected)
                        autobalance_msg += format(" IPC ID ", ipc::peer->client_id, "\"");
                    else
                        autobalance_msg += "\"";
                    g_IEngine->ClientCmd_Unrestricted(autobalance_msg.c_str());
                }
                anti_balance_attempts++;
            }
        }
        break;
    }
    case 4:
    {
        s = buf.GetNumBytesLeft();
        if (s >= 256 || CE_BAD(LOCAL_E))
            break;

        for (i = 0; i < s; i++)
            data.push_back(buf_data[i]);
        /* First byte is player ENT index
         * Second byte is unindentified (equals to 0x01)
         */
        const char *p = data.c_str() + 2;
        std::string event(p), name((p += event.size() + 1)), message(p + name.size() + 1);
        if (chat_filter_enable && data[0] == LOCAL_E->m_IDX && event == "#TF_Name_Change")
        {
            chat_stack::Say("\e" + clear, false);
        }
        else if (chat_filter_enable && data[0] != LOCAL_E->m_IDX && event.find("TF_Chat") == 0)
        {
            player_info_s info{};
            g_IEngine->GetPlayerInfo(LOCAL_E->m_IDX, &info);
            const char *claz  = nullptr;
            std::string name1 = info.name;

            switch (g_pLocalPlayer->clazz)
            {
            case tf_scout:
                claz = "scout";
                break;
            case tf_soldier:
                claz = "soldier";
                break;
            case tf_pyro:
                claz = "pyro";
                break;
            case tf_demoman:
                claz = "demo";
                break;
            case tf_engineer:
                claz = "engi";
                break;
            case tf_heavy:
                claz = "heavy";
                break;
            case tf_medic:
                claz = "med";
                break;
            case tf_sniper:
                claz = "sniper";
                break;
            case tf_spy:
                claz = "spy";
                break;
            default:
                break;
            }

            std::vector<std::string> res = { "skid", "script", "cheat", "hak", "hac", "f1", "hax", "vac", "ban", "bot", "report", "kick" };
            if (claz)
                res.emplace_back(claz);

            SplitName(res, name1, 2);
            SplitName(res, name1, 3);

            std::string message2(message);
            boost::to_lower(message2);

            const char *toreplace[]   = { " ", "4", "3", "0", "6", "5", "7" };
            const char *replacewith[] = { "", "a", "e", "o", "g", "s", "t" };

            for (int i = 0; i < 7; i++)
                boost::replace_all(message2, toreplace[i], replacewith[i]);

            for (auto filter : res)
                if (boost::contains(message2, filter))
                {
                    chat_stack::Say("\e" + clear, true);
                    retrun     = true;
                    lastfilter = message;
                    lastname   = format(name);
                    gitgud.update();
                    break;
                }
        }
        if (event.find("TF_Chat") == 0)
            hacks::shared::ChatCommands::handleChatMessage(message, data[0]);
        chatlog::LogMessage(data[0], message);
        buf = bf_read(data.c_str(), data.size());
        buf.Seek(0);
        break;
    }
    }

    if (dispatch_log)
    {
        logging::Info("D> %i", type);
        std::ostringstream str;
        while (buf.GetNumBytesLeft())
            str << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buf.ReadByte()) << ' ';

        std::string msg(str.str());
        logging::Info("MESSAGE %d, DATA = [ %s ] strings listed below", type, msg.c_str());
        buf.Seek(0);

        i = 0;
        msg.clear();
        while (buf.GetNumBytesLeft())
        {
            if ((c = buf.ReadByte()))
                msg.push_back(c);
            else
            {
                logging::Info("[%d] %s", i++, msg.c_str());
                msg.clear();
            }
        }
        buf.Seek(0);
    }
    votelogger::dispatchUserMessage(buf, type);
    return original::DispatchUserMessage(this_, type, buf);
}
} // namespace hooked_methods
