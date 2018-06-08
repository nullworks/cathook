/*
 * votelogger.cpp
 *
 *  Created on: Dec 31, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <boost/algorithm/string.hpp>

namespace votelogger
{

static CatVar enabled(CV_SWITCH, XORSTR("votelog"), XORSTR("0"), XORSTR("Log votes"));
static CatVar requeue(CV_SWITCH, XORSTR("votelog_requeue"), XORSTR("1"),
                      XORSTR("Auto requeue on vote kick"), XORSTR("Auto requeue on vote kick"));
static CatVar anti_votekick(CV_SWITCH, XORSTR("anti_votekick"), XORSTR("0"), XORSTR("anti-votekick"),
                            XORSTR("Prevent votekicks by lagging the server in a way ")
                            XORSTR("that every vote comes is delayed.\ndo not forget ")
                            XORSTR("to enable votelog and that this\nmakes the server ")
                            XORSTR("be down for about 30 seconds\ncl_timeout 60 is a ")
                            XORSTR("must"));
static CatVar kick_msg(CV_STRING, XORSTR("anti_votekick_string"),
                       XORSTR("Everyone thank $NAME for initiating the votekick! The ")
                       XORSTR("server will now be shut down!"),
                       XORSTR("anti-votekick message"),
                       XORSTR("Send this message on Votekick attempts against ")
                       XORSTR("you.\n$NAME gets replaced with their name\n$CLASS with ")
                       XORSTR("their class."));
Timer antikick{};
bool active = false;

const std::string tf_classes[] = { XORSTR("class"),   XORSTR("scout"),   XORSTR("sniper"), XORSTR("soldier"),
                                   XORSTR("demoman"), XORSTR("medic"),   XORSTR("heavy"),  XORSTR("pyro"),
                                   XORSTR("spy"),     XORSTR("engineer") };
void user_message(bf_read &buffer, int type)
{

    bool islocalplayer = false;
    if (!enabled)
        return;
    switch (type)
    {
    case 45:
        // Call Vote Failed
        break;
    case 46:
    {
        islocalplayer        = false;
        unsigned char caller = buffer.ReadByte();
        // unknown
        buffer.ReadByte();
        char reason[64];
        char name[64];
        buffer.ReadString(reason, 64, false, nullptr);
        buffer.ReadString(name, 64, false, nullptr);
        unsigned char eid = buffer.ReadByte();
        buffer.Seek(0);
        eid >>= 1;

        unsigned steamID = 0;
        player_info_s info;
        if (g_IEngine->GetPlayerInfo(eid, &info))
            steamID = info.friendsID;
        if (eid == LOCAL_E->m_IDX ||
            playerlist::AccessData(steamID).state ==
                playerlist::k_EState::FRIEND)
        {
            islocalplayer = true;
            if (anti_votekick && !active)
            {
                active = true;
                antikick.update();
                std::string msg(kick_msg.GetString());
                ReplaceString(msg, XORSTR("$NAME"), format(info.name));
                if (CE_GOOD(ENTITY(eid)))
                {
                    int clz = g_pPlayerResource->GetClass(ENTITY(eid));
                    ReplaceString(msg, XORSTR("$CLASS"), format(tf_classes[clz]));
                }
                NET_StringCmd senddata(format(XORSTR("say "), msg).c_str());
                INetChannel *ch =
                    (INetChannel *) g_IEngine->GetNetChannelInfo();
                senddata.SetNetChannel(ch);
                senddata.SetReliable(true);
                ch->SendNetMsg(senddata, true);
                ch->Transmit();
            }
        }

        logging::Info(XORSTR("Vote called to kick %s [U:1:%u] for %s"), name, steamID,
                      reason);
        break;
    }
    case 47:
        logging::Info(XORSTR("Vote passed"));
        if (islocalplayer && requeue)
            tfmm::queue_start();
        break;
    case 48:
        logging::Info(XORSTR("Vote failed"));
        break;
    case 49:
        logging::Info(XORSTR("VoteSetup?"));
        break;
    }
}
}
