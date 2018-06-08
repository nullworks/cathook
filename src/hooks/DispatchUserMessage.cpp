/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <chatlog.hpp>
#include <ucccccp.hpp>
#include <boost/algorithm/string.hpp>
#include <MiscTemporary.hpp>
#include <hacks/AntiAim.hpp>
#include "HookedMethods.hpp"

static bool retrun = false;
static Timer sendmsg{};
static Timer gitgud{};
static CatVar clean_chat(CV_SWITCH, XORSTR("clean_chat"), XORSTR("0"), XORSTR("Clean chat"),
                         XORSTR("Removes newlines from chat"));
static CatVar dispatch_log(CV_SWITCH, XORSTR("debug_log_usermessages"), XORSTR("0"),
                           XORSTR("Log dispatched user messages"));
static CatVar chat_filter(CV_STRING, XORSTR("chat_censor"), XORSTR(""), XORSTR("Censor words"),
                          XORSTR("Spam Chat with newlines if the chosen words are "
                          "said, seperate with commas"));
static CatVar chat_filter_enabled(CV_SWITCH, XORSTR("chat_censor_enabled"), XORSTR("0"),
                                  XORSTR("Enable censor"), XORSTR("Censor Words in chat"));
std::string clear = XORSTR("");
std::string lastfilter{};
std::string lastname{};

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(DispatchUserMessage, bool, void *this_, int type,
                     bf_read &buf)
{
    if (retrun && gitgud.test_and_set(10000))
    {
        PrintChat(XORSTR("\x07%06X%s\x01: %s"), 0xe05938, lastname.c_str(),
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
            for (i      = 0; i < s; i++)
                data[i] = buf.ReadByte();
            j           = 0;
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
            if (chat_filter_enabled && data[0] != LOCAL_E->m_IDX)
            {
                if (!strcmp(chat_filter.GetString(), XORSTR("")))
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
                        claz = XORSTR("scout");
                        break;
                    case tf_soldier:
                        claz = XORSTR("soldier");
                        break;
                    case tf_pyro:
                        claz = XORSTR("pyro");
                        break;
                    case tf_demoman:
                        claz = XORSTR("demo");
                        break;
                    case tf_engineer:
                        claz = XORSTR("engi");
                        break;
                    case tf_heavy:
                        claz = XORSTR("heavy");
                        break;
                    case tf_medic:
                        claz = XORSTR("med");
                        break;
                    case tf_sniper:
                        claz = XORSTR("sniper");
                        break;
                    case tf_spy:
                        claz = XORSTR("spy");
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
                            tmp = XORSTR("");
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
                            tmp2 = XORSTR("");
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
                        XORSTR("skid"), XORSTR("script"), XORSTR("cheat"), XORSTR("hak"),   XORSTR("hac"),  XORSTR("f1"),
                        XORSTR("hax"),  XORSTR("vac"),    XORSTR("ban"),   XORSTR("lmao"),  XORSTR("bot"),  XORSTR("report"),
                        XORSTR("cat"),  XORSTR("insta"),  XORSTR("revv"),  XORSTR("brass"), XORSTR("kick"), claz
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
                    boost::replace_all(message2, XORSTR("4"), XORSTR("a"));
                    boost::replace_all(message2, XORSTR("3"), XORSTR("e"));
                    boost::replace_all(message2, XORSTR("0"), XORSTR("o"));
                    boost::replace_all(message2, XORSTR("6"), XORSTR("g"));
                    boost::replace_all(message2, XORSTR("5"), XORSTR("s"));
                    boost::replace_all(message2, XORSTR("7"), XORSTR("t"));
                    for (auto filter : res)
                    {
                        if (retrun)
                            break;
                        if (boost::contains(message2, filter))
                        {

                            if (clear == XORSTR(""))
                            {
                                for (int i = 0; i < 120; i++)
                                    clear += XORSTR("\n");
                            }
                            *bSendPackets = true;
                            chat_stack::Say(XORSTR(". ") + clear, true);
                            retrun     = true;
                            lastfilter = format(filter);
                            lastname   = format(name);
                        }
                    }
                }
                else if (data[0] != LOCAL_E->m_IDX)
                {
                    std::string input = chat_filter.GetString();
                    boost::to_lower(input);
                    std::string message2 = message;
                    std::vector<std::string> result{};
                    boost::split(result, input, boost::is_any_of(XORSTR(",")));
                    boost::replace_all(message2, XORSTR("4"), XORSTR("a"));
                    boost::replace_all(message2, XORSTR("3"), XORSTR("e"));
                    boost::replace_all(message2, XORSTR("0"), XORSTR("o"));
                    boost::replace_all(message2, XORSTR("6"), XORSTR("g"));
                    boost::replace_all(message2, XORSTR("5"), XORSTR("s"));
                    boost::replace_all(message2, XORSTR("7"), XORSTR("t"));
                    for (auto filter : result)
                    {
                        if (retrun)
                            break;
                        if (boost::contains(message2, filter))
                        {
                            if (clear == XORSTR(""))
                            {
                                clear = XORSTR("");
                                for (int i = 0; i < 120; i++)
                                    clear += XORSTR("\n");
                            }
                            *bSendPackets = true;
                            chat_stack::Say(XORSTR(". ") + clear, true);
                            retrun     = true;
                            lastfilter = format(filter);
                            lastname   = format(name);
                        }
                    }
                }
            }
#if not LAGBOT_MODE
            if (sendmsg.test_and_set(300000) &&
                hacks::shared::antiaim::communicate)
                chat_stack::Say(XORSTR("!!meow"));
#endif
            if (crypt_chat)
            {
                if (message.find(XORSTR("!!")) == 0)
                {
                    if (ucccccp::validate(message))
                    {
#if not LAGBOT_MODE
                        if (ucccccp::decrypt(message) == XORSTR("meow") &&
                            hacks::shared::antiaim::communicate &&
                            data[0] != LOCAL_E->m_IDX &&
                            playerlist::AccessData(ENTITY(data[0])).state !=
                                playerlist::k_EState::CAT)
                        {
                            playerlist::AccessData(ENTITY(data[0])).state =
                                playerlist::k_EState::CAT;
                            chat_stack::Say(XORSTR("!!meow"));
                        }
#endif
                        PrintChat(XORSTR("\x07%06X%s\x01: %s"), 0xe05938, name.c_str(),
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
        logging::Info(XORSTR("D> %i"), type);
        std::ostringstream str{};
        while (buf.GetNumBytesLeft())
        {
            unsigned char byte = buf.ReadByte();
            str << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte) << ' ';
        }
        logging::Info(XORSTR("MESSAGE %d, DATA = [ %s ]"), type, str.str().c_str());
        buf.Seek(0);
    }
    votelogger::user_message(buf, type);
    return original::DispatchUserMessage(this_, type, buf);
}
}
