/*
 * netmessage.h
 *
 *  Created on: Dec 3, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <bitbuf.h>
#include <utlvector.h>
#include <inetchannel.h>
#include <inetmessage.h>
#include <inetmsghandler.h>
#include <KeyValues.h>
#include "bitvec.h"
#include "CSignature.h"

#define DECLARE_BASE_MESSAGE(msgtype)     \
public:                                   \
    bool ReadFromBuffer(bf_read &buffer); \
    bool WriteToBuffer(bf_write &buffer); \
    const char *ToString() const;         \
    int GetType() const                   \
    {                                     \
        return msgtype;                   \
    }                                     \
    const char *GetName() const           \
    {                                     \
        return #msgtype;                  \
    }

#define DECLARE_NET_MESSAGE(name)          \
    DECLARE_BASE_MESSAGE(net_##name);      \
    INetMessageHandler *m_pMessageHandler; \
    bool Process()                         \
    {                                      \
        return false;                      \
    }

#define DECLARE_SVC_MESSAGE(name)                      \
    DECLARE_BASE_MESSAGE(svc_##name);                  \
    IServerMessageHandler *m_pMessageHandler;          \
    bool Process()                                     \
    {                                                  \
        return m_pMessageHandler->Process##name(this); \
    }

#define DECLARE_CLC_MESSAGE(name)                      \
    DECLARE_BASE_MESSAGE(clc_##name);                  \
    IClientMessageHandler *m_pMessageHandler;          \
    bool Process()                                     \
    {                                                  \
        return m_pMessageHandler->Process##name(this); \
    }

#define DECLARE_MM_MESSAGE(name)                       \
    DECLARE_BASE_MESSAGE(mm_##name);                   \
    IMatchmakingMessageHandler *m_pMessageHandler;     \
    bool Process()                                     \
    {                                                  \
        return m_pMessageHandler->Process##name(this); \
    }

class CNetMessage : public INetMessage
{
public:
    CNetMessage()
    {
        m_bReliable  = true;
        m_NetChannel = 0;
    }

    virtual ~CNetMessage(){};

    virtual int GetGroup() const
    {
        return INetChannelInfo::GENERIC;
    }
    INetChannel *GetNetChannel() const
    {
        return m_NetChannel;
    }

    virtual void SetReliable(bool state)
    {
        m_bReliable = state;
    };
    virtual bool IsReliable() const
    {
        return m_bReliable;
    };
    virtual void SetNetChannel(INetChannel *netchan)
    {
        m_NetChannel = netchan;
    }
    virtual bool Process()
    {
        return false;
    }; // no handler set

public:
    // I don't get what it does but we need it
    virtual bool BIncomingMessageForProcessing(double param_1, int param_2)
    {
        // Call original to be sure nothing breaks
        typedef bool (*BIncomingMessageForProcessing_t)(CNetMessage *, double, int);
        static auto addr = gSignatures.GetEngineSignature("55 89 E5 56 53 83 EC 10 8B 5D ? F2 0F 10 45");
        BIncomingMessageForProcessing_t BIncomingMessageForProcessing_fn = (BIncomingMessageForProcessing_t)addr;
        return BIncomingMessageForProcessing_fn(this, param_1, param_2);
    };
    // I don't get what it does but we need it
    virtual void SetRatePolicy()
    {
        // Call original to be sure nothing breaks
        typedef bool (*SetRatePolicy_t)(CNetMessage *);
        static auto addr = gSignatures.GetEngineSignature("55 89 E5 83 EC 18 C7 04 24 2C 00 00 00");
        SetRatePolicy_t SetRatePolicy_fn = (SetRatePolicy_t)addr;
        SetRatePolicy_fn(this);
    };

protected:
    bool m_bReliable;          // true if message should be send reliable
    INetChannel *m_NetChannel; // netchannel this message is from/for
};

#define net_NOP 0        // nop command used for padding
#define net_Disconnect 1 // disconnect, last message in connection
#define net_File 2       // file transmission message request/deny

#define net_Tick 3        // send last world tick
#define net_StringCmd 4   // a string command
#define net_SetConVar 5   // sends one/multiple convar settings
#define net_SignonState 6 // signals current signon state

//
// server to client
//

#define svc_Print 7      // print text to console
#define svc_ServerInfo 8 // first message from server about game, map etc
#define svc_SendTable 9  // sends a sendtable description for a game class
#define svc_ClassInfo 10 // Info about classes (first byte is a CLASSINFO_ define).
#define svc_SetPause 11  // tells client if server paused or unpaused

#define svc_CreateStringTable 12 // inits shared string tables
#define svc_UpdateStringTable 13 // updates a string table

#define svc_VoiceInit 14 // inits used voice codecs & quality
#define svc_VoiceData 15 // Voicestream data from the server

// #define svc_HLTV			16		// HLTV control messages

#define svc_Sounds 17 // starts playing sound

#define svc_SetView 18        // sets entity as point of view
#define svc_FixAngle 19       // sets/corrects players viewangle
#define svc_CrosshairAngle 20 // adjusts crosshair in auto aim mode to lock on traget

#define svc_BSPDecal 21 // add a static decal to the worl BSP
// NOTE: This is now unused!
//#define	svc_TerrainMod		22		// modification to the
// terrain/displacement

// Message from server side to client side entity
#define svc_UserMessage 23   // a game specific message
#define svc_EntityMessage 24 // a message for an entity
#define svc_GameEvent 25     // global game event fired

#define svc_PacketEntities 26 // non-delta compressed entities

#define svc_TempEntities 27 // non-reliable event object

#define svc_Prefetch 28 // only sound indices for now

#define svc_Menu 29 // display a menu from a plugin

#define svc_GameEventList 30 // list of known games events and fields

#define svc_GetCvarValue 31 // Server wants to know the value of a cvar on the client.

#define SVC_LASTMSG 31 // last known server messages

//
// client to server
//

#define clc_ClientInfo 8        // client info (table CRC etc)
#define clc_Move 9              // [CUserCmd]
#define clc_VoiceData 10        // Voicestream data from a client
#define clc_BaselineAck 11      // client acknowledges a new baseline seqnr
#define clc_ListenEvents 12     // client acknowledges a new baseline seqnr
#define clc_RespondCvarValue 13 // client is responding to a svc_GetCvarValue message.
#define clc_FileCRCCheck 14     // client is sending a file's CRC to the server to be verified.
#define clc_CmdKeyValues 16     // client gets sent some update

#define CLC_LASTMSG 14 //	last known client message

#define MAX_OSPATH 260

#define NETMSG_TYPE_BITS 6
typedef int QueryCvarCookie_t;
typedef enum
{
    eQueryCvarValueStatus_ValueIntact   = 0, // It got the value fine.
    eQueryCvarValueStatus_CvarNotFound  = 1,
    eQueryCvarValueStatus_NotACvar      = 2, // There's a ConCommand, but it's not a ConVar.
    eQueryCvarValueStatus_CvarProtected = 3  // The cvar was marked with FCVAR_SERVER_CAN_NOT_QUERY, so the server
                                             // is not allowed to have its value.
} EQueryCvarValueStatus;

class CLC_RespondCvarValue : public CNetMessage
{
public:
    DECLARE_CLC_MESSAGE(RespondCvarValue);

    QueryCvarCookie_t m_iCookie;

    const char *m_szCvarName;
    const char *m_szCvarValue; // The sender sets this, and it automatically
                               // points it at m_szCvarNameBuffer when
                               // receiving.

    EQueryCvarValueStatus m_eStatusCode;

private:
    char m_szCvarNameBuffer[256];
    char m_szCvarValueBuffer[256];
};

class NET_NOP : public CNetMessage
{
    DECLARE_NET_MESSAGE(NOP);

    int GetGroup() const
    {
        return INetChannelInfo::GENERIC;
    }
    NET_NOP(){};
};

class NET_SignonState : public CNetMessage
{
    DECLARE_NET_MESSAGE(SignonState);

    int GetGroup() const
    {
        return INetChannelInfo::SIGNON;
    }

    NET_SignonState(){};
    NET_SignonState(int state, int spawncount)
    {
        m_nSignonState = state;
        m_nSpawnCount  = spawncount;
    };

public:
    int m_nSignonState; // See SIGNONSTATE_ defines
    int m_nSpawnCount;  // server spawn count (session number)
};

class SVC_GetCvarValue : public CNetMessage
{
public:
    DECLARE_SVC_MESSAGE(GetCvarValue);

    QueryCvarCookie_t m_iCookie;
    const char *m_szCvarName; // The sender sets this, and it automatically
                              // points it at m_szCvarNameBuffer when receiving.

private:
    char m_szCvarNameBuffer[256];
};

class CLC_Move : public CNetMessage
{
    DECLARE_CLC_MESSAGE(Move);

    int GetGroup() const
    {
        return INetChannelInfo::MOVE;
    }

    CLC_Move()
    {
        m_bReliable = false;
    }

public:
    int m_nBackupCommands;
    int m_nNewCommands;
    int m_nLength;
    bf_read m_DataIn;
    bf_write m_DataOut;
};

class CLC_VoiceData : public CNetMessage
{
    DECLARE_CLC_MESSAGE(VoiceData);

    int GetGroup() const
    {
        return INetChannelInfo::VOICE;
    }

    CLC_VoiceData()
    {
        m_bReliable = false;
    };

public:
    int m_nLength;
    bf_read m_DataIn;
    bf_write m_DataOut;
    uint64 m_xuid;
};

class CLC_BaselineAck : public CNetMessage
{
    DECLARE_CLC_MESSAGE(BaselineAck);

    CLC_BaselineAck(){};
    CLC_BaselineAck(int tick, int baseline)
    {
        m_nBaselineTick = tick;
        m_nBaselineNr   = baseline;
    }

    int GetGroup() const
    {
        return INetChannelInfo::ENTITIES;
    }

public:
    int m_nBaselineTick; // sequence number of baseline
    int m_nBaselineNr;   // 0 or 1
};

class CLC_ListenEvents : public CNetMessage
{
    DECLARE_CLC_MESSAGE(ListenEvents);

    int GetGroup() const
    {
        return INetChannelInfo::SIGNON;
    }

public:
    CBitVec<1 << 9> m_EventArray;
};

class CLC_CmdKeyValues : public CNetMessage
{
    DECLARE_CLC_MESSAGE(CmdKeyValues);

    int GetGroup() const
    {
        return INetChannelInfo::GENERIC;
    };
    CLC_CmdKeyValues()
    {
        m_bReliable = false;
    };

public:
};

class NET_SetConVar : public CNetMessage
{
    DECLARE_NET_MESSAGE(SetConVar);

    int GetGroup() const
    {
        return INetChannelInfo::STRINGCMD;
    }

    NET_SetConVar()
    {
    }
    NET_SetConVar(const char *name, const char *value)
    {
        cvar_t cvar;
        strncpy(cvar.name, name, MAX_OSPATH);
        strncpy(cvar.value, value, MAX_OSPATH);
        convar = cvar;
    }

public:
    typedef struct cvar_s
    {
        char name[MAX_OSPATH];
        char value[MAX_OSPATH];
    } cvar_t;
    cvar_t convar;
    // CUtlVector<cvar_t> m_ConVars;
};

class NET_StringCmd : public CNetMessage
{
    DECLARE_NET_MESSAGE(StringCmd);

    int GetGroup() const
    {
        return INetChannelInfo::STRINGCMD;
    }

    NET_StringCmd()
    {
        m_szCommand = NULL;
    };
    NET_StringCmd(const char *cmd)
    {
        m_szCommand = cmd;
    };

public:
    const char *m_szCommand; // execute this command

private:
    char m_szCommandBuffer[1024]; // buffer for received messages
};
