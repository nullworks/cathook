/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <ucccccp.hpp>
#include <MiscTemporary.hpp>
#include "HookedMethods.hpp"

static CatVar newlines_msg(CV_INT, XORSTR("chat_newlines"), XORSTR("0"), XORSTR("Prefix newlines"),
                           XORSTR("Add # newlines before each your message"), 0, 24);

static CatVar log_sent(CV_SWITCH, XORSTR("debug_log_sent_messages"), XORSTR("0"),
                       XORSTR("Log sent messages"));

namespace hooked_methods
{
DEFINE_HOOKED_METHOD(SendNetMsg, bool, INetChannel *this_, INetMessage &msg,
                     bool force_reliable, bool voice)
{
    size_t say_idx, say_team_idx;
    int offset;
    std::string newlines;
    NET_StringCmd stringcmd;
    // net_StringCmd
    if (msg.GetType() == 4 && (newlines_msg || crypt_chat))
    {
        std::string str(msg.ToString());
        say_idx      = str.find(XORSTR("net_StringCmd: \"say \""));
        say_team_idx = str.find(XORSTR("net_StringCmd: \"say_team \""));
        if (!say_idx || !say_team_idx)
        {
            offset    = say_idx ? 26 : 21;
            bool crpt = false;
            if (crypt_chat)
            {
                std::string msg(str.substr(offset));
                msg = msg.substr(0, msg.length() - 2);
                if (msg.find(XORSTR("!!")) == 0)
                {
                    msg  = ucccccp::encrypt(msg.substr(2));
                    str  = str.substr(0, offset) + msg + XORSTR("\"\"");
                    crpt = true;
                }
            }
            if (!crpt && newlines_msg)
            {
                // TODO move out? update in a value change callback?
                newlines = std::string((int) newlines_msg, '\n');
                str.insert(offset, newlines);
            }
            str = str.substr(16, str.length() - 17);
            // if (queue_messages && !chat_stack::CanSend()) {
            stringcmd.m_szCommand = str.c_str();
            return original::SendNetMsg(this_, stringcmd, force_reliable,
                                        voice);
            //}
        }
    }
    static ConVar *sv_player_usercommand_timeout =
        g_ICvar->FindVar(XORSTR("sv_player_usercommand_timeout"));
    static float lastcmd = 0.0f;
    if (lastcmd > g_GlobalVars->absoluteframetime)
    {
        lastcmd = g_GlobalVars->absoluteframetime;
    }
    if (log_sent && msg.GetType() != 3 && msg.GetType() != 9)
    {
        logging::Info(XORSTR("=> %s [%i] %s"), msg.GetName(), msg.GetType(),
                      msg.ToString());
        unsigned char buf[4096];
        bf_write buffer(XORSTR("cathook_debug_buffer"), buf, 4096);
        logging::Info(XORSTR("Writing %i"), msg.WriteToBuffer(buffer));
        std::string bytes    = XORSTR("");
        constexpr char h2c[] = XORSTR("0123456789abcdef");
        for (int i = 0; i < buffer.GetNumBytesWritten(); i++)
        {
            // bytes += format(h2c[(buf[i] & 0xF0) >> 4], h2c[(buf[i] & 0xF)], '
            // ');
            bytes += format((unsigned short) buf[i], ' ');
        }
        logging::Info(XORSTR("%i bytes => %s"), buffer.GetNumBytesWritten(),
                      bytes.c_str());
    }
    return original::SendNetMsg(this_, msg, force_reliable, voice);
}
}
