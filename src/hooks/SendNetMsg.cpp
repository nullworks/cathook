/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <ucccccp.hpp>
#include <MiscTemporary.hpp>
#include <settings/Int.hpp>
#include "HookedMethods.hpp"
#include <MiscTemporary.hpp>

static settings::Int newlines_msg{ "chat.prefix-newlines", "0" };
static settings::Bool log_sent{ "debug.log-sent-chat", "false" };

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
        say_idx      = str.find("net_StringCmd: \"say \"");
        say_team_idx = str.find("net_StringCmd: \"say_team \"");
        if (!say_idx || !say_team_idx)
        {
            offset    = say_idx ? 26 : 21;
            bool crpt = false;
            if (crypt_chat)
            {
                std::string msg(str.substr(offset));
                msg = msg.substr(0, msg.length() - 2);
                if (msg.find("!!") == 0)
                {
                    // Change this version if you want version A back, look at
                    // ucccccp docs for more info
                    char Version = 'B';
                    msg          = ucccccp::encrypt(msg.substr(2), Version);
                    str          = str.substr(0, offset) + msg + "\"\"";
                    crpt         = true;
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
        g_ICvar->FindVar("sv_player_usercommand_timeout");
    static float lastcmd = 0.0f;
    if (lastcmd > g_GlobalVars->absoluteframetime)
    {
        lastcmd = g_GlobalVars->absoluteframetime;
    }
    if (log_sent && msg.GetType() != 3 && msg.GetType() != 9)
    {
        logging::Info("=> %s [%i] %s", msg.GetName(), msg.GetType(),
                      msg.ToString());
        unsigned char buf[4096];
        bf_write buffer("cathook_debug_buffer", buf, 4096);
        logging::Info("Writing %i", msg.WriteToBuffer(buffer));
        std::string bytes    = "";
        constexpr char h2c[] = "0123456789abcdef";
        for (int i = 0; i < buffer.GetNumBytesWritten(); i++)
        {
            // bytes += format(h2c[(buf[i] & 0xF0) >> 4], h2c[(buf[i] & 0xF)], '
            // ');
            bytes += format((unsigned short) buf[i], ' ');
        }
        logging::Info("%i bytes => %s", buffer.GetNumBytesWritten(),
                      bytes.c_str());
    }
    return original::SendNetMsg(this_, msg, force_reliable, voice);
}
} // namespace hooked_methods
