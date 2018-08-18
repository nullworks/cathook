/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <chatlog.hpp>
#include <ucccccp.hpp>
#include <boost/algorithm/string.hpp>
#include <MiscTemporary.hpp>
#include <hacks/AntiAim.hpp>
#include <settings/Bool.hpp>
#include "HookedMethods.hpp"

static settings::Bool clean_chat{ "chat.clean", "false" };
static settings::Bool dispatch_log{ "debug.log-dispatch-user-msg", "false" };
static settings::String chat_filter{ "chat.censor.filter", "" };
static settings::Bool chat_filter_enable{ "chat.censor.enable", "false" };

static bool retrun = false;
static Timer sendmsg{};
static Timer gitgud{};

const std::string clear = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                          "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                          "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                          "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                          "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
std::string lastfilter{};
std::string lastname{};

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(DispatchUserMessage, bool, void *this_, int type,
                     bf_read &buf)
{
    if (retrun && gitgud.test_and_set(100))
    {
        PrintChat("\x07%06X%s\x01: %s", 0xe05938, lastname.c_str(),
                  lastfilter.c_str());
        retrun = false;
    }
    int loop_index, s, i, j;
    char *data, c;

    if (type == 4)
    {
        loop_index = 0;
        s          = buf.GetNumBytesLeft();
        if (s < 256)
        {
            data = (char *) alloca(s);
            for (i = 0; i < s; i++)
                data[i] = buf.ReadByte();
            j = 0;
            std::string name;
            std::string message;
            for (i = 0; i < 3; i++)
            {
                while ((c = data[j++]) && (loop_index < 128))
                {
                    loop_index++;
                    if (clean_chat)
                        if ((c == '\n' || c == '\r') && (i == 1 || i == 2))
                            data[j - 1] = '*';
                    if (i == 1)
                        name.push_back(c);
                    if (i == 2)
                        message.push_back(c);
                }
            }
            if (chat_filter_enable && data[0] != LOCAL_E->m_IDX)
            {
                if (sizeof(*chat_filter) < 10)
                {
                    std::string tmp  = {};
                    std::string tmp2 = {};
                    int iii          = 0;
                    player_info_s info;
                    g_IEngine->GetPlayerInfo(LOCAL_E->m_IDX, &info);
                    std::string name1 = info.name;
                    std::vector<std::string> name2{};
                    std::vector<std::string> name3{};
                    std::string claz = {};
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
                    for (char i : name1)
                    {
                        if (iii == 2)
                        {
                            iii = 0;
                            tmp += i;
                            name2.push_back(tmp);
                            tmp = "";
                        }
                        else if (iii < 2)
                        {
                            iii++;
                            tmp += i;
                        }
                    }
                    iii = 0;
                    for (char i : name1)
                    {
                        if (iii == 3)
                        {
                            iii = 0;
                            tmp += i;
                            name3.push_back(tmp2);
                            tmp2 = "";
                        }
                        else if (iii < 3)
                        {
                            iii++;
                            tmp2 += i;
                        }
                    }
                    if (tmp.size() > 2)
                        name2.push_back(tmp);
                    if (tmp2.size() > 2)
                        name3.push_back(tmp2);
                    iii                          = 0;
                    std::vector<std::string> res = {
                        "skid", "script", "cheat", "hak",   "hac",  "f1",
                        "hax",  "vac",    "ban",   "lmao",  "bot",  "report",
                        "cat",  "insta",  "revv",  "brass", "kick", claz
                    };
                    for (auto i : name2)
                    {
                        boost::to_lower(i);
                        res.push_back(i);
                    }
                    for (auto i : name3)
                    {
                        boost::to_lower(i);
                        res.push_back(i);
                    }
                    std::string message2 = message;
                    boost::to_lower(message2);
                    boost::replace_all(message2, "4", "a");
                    boost::replace_all(message2, "3", "e");
                    boost::replace_all(message2, "0", "o");
                    boost::replace_all(message2, "6", "g");
                    boost::replace_all(message2, "5", "s");
                    boost::replace_all(message2, "7", "t");
                    for (auto filter : res)
                    {
                        if (boost::contains(message2, filter))
                        {
                            *bSendPackets = true;
                            chat_stack::Say(". " + clear, true);
                            retrun     = true;
                            lastfilter = format(filter);
                            lastname   = format(name);
			    gitgud.update();
                        }
                    }
                }
                else if (data[0] != LOCAL_E->m_IDX)
                {
                    std::string input = *chat_filter;
                    boost::to_lower(input);
                    std::string message2 = message;
                    std::vector<std::string> result{};
                    boost::split(result, input, boost::is_any_of(","));
                    boost::replace_all(message2, "4", "a");
                    boost::replace_all(message2, "3", "e");
                    boost::replace_all(message2, "0", "o");
                    boost::replace_all(message2, "6", "g");
                    boost::replace_all(message2, "5", "s");
                    boost::replace_all(message2, "7", "t");
                    for (auto filter : result)
                    {
                        if (retrun)
                            break;
                        if (boost::contains(message2, filter))
                        {
                            *bSendPackets = true;
                            chat_stack::Say(". " + clear, true);
                            retrun     = true;
                            lastfilter = format(filter);
                            lastname   = format(name);
			    gitgud.update();
                        }
                    }
                }
            }
#if !LAGBOT_MODE
/*if (sendmsg.test_and_set(300000) &&
    hacks::shared::antiaim::communicate)
    chat_stack::Say("!!meow");*/
#endif
            if (crypt_chat)
            {
                if (message.find("!!B") == 0)
                {
                    if (ucccccp::validate(message))
                    {
#if !LAGBOT_MODE
/*                        if (ucccccp::decrypt(message) == "meow" &&
                            hacks::shared::antiaim::communicate &&
                            data[0] != LOCAL_E->m_IDX &&
                            playerlist::AccessData(ENTITY(data[0])).state !=
                                playerlist::k_EState::CAT)
                        {
                            playerlist::AccessData(ENTITY(data[0])).state =
                                playerlist::k_EState::CAT;
                            chat_stack::Say("!!meow");
                        }*/
#endif
                        PrintChat("\x07%06X%s\x01: %s", 0xe05938, name.c_str(),
                                  ucccccp::decrypt(message).c_str());
                    }
                }
            }
            chatlog::LogMessage(data[0], message);
            buf = bf_read(data, s);
            buf.Seek(0);
        }
    }
    if (dispatch_log)
    {
        logging::Info("D> %i", type);
        std::ostringstream str{};
        while (buf.GetNumBytesLeft())
        {
            unsigned char byte = buf.ReadByte();
            str << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte) << ' ';
        }
        logging::Info("MESSAGE %d, DATA = [ %s ]", type, str.str().c_str());
        buf.Seek(0);
    }
    votelogger::dispatchUserMessage(buf, type);
    return original::DispatchUserMessage(this_, type, buf);
}
} // namespace hooked_methods
