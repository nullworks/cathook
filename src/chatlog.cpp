/*
 * chatlog.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "init.hpp"

#include <pwd.h>
#include <unistd.h>
#include <hacks/Spam.hpp>

namespace chatlog
{

static CatVar enabled(CV_SWITCH, XORSTR("chat_log"), XORSTR("0"), XORSTR("Chat log"),
                      XORSTR("Log chat to file"));
static CatVar dont_log_spam(CV_SWITCH, XORSTR("chat_log_nospam"), XORSTR("1"), XORSTR("No Spam"),
                            XORSTR("Don't log your messages if spam is active"));
static CatVar dont_log_ipc(CV_SWITCH, XORSTR("chat_log_noipc"), XORSTR("1"), XORSTR("No IPC"),
                           XORSTR("Don't log messages sent by bots"));

class csv_stream
{
public:
    struct end_t
    {
    };
    static end_t end;

public:
    csv_stream()
    {
        open();
    }
    ~csv_stream()
    {
        stream.close();
    }
    bool open()
    {
        logging::Info(XORSTR("csvstream: Trying to open log file"));
        uid_t uid         = geteuid();
        struct passwd *pw = getpwuid(uid);
        std::string uname = XORSTR("");
        if (pw)
        {
            uname = std::string(pw->pw_name);
        }
        stream.open(DATA_PATH XORSTR("/chat-") + uname + XORSTR(".csv"),
                    std::ios::out | std::ios::app);
        return stream.good();
    }

public:
    int columns{ 0 };
    std::ofstream stream;
};

csv_stream::end_t csv_stream::end{};

csv_stream &operator<<(csv_stream &log, const std::string &string)
{
    if (!log.stream.good())
    {
        logging::Info(XORSTR("[ERROR] csvstream is not open!"));
        if (!log.open())
            return log;
    }
    if (log.columns)
        log.stream << ',';
    log.stream << 'XORSTR("';
    for (const auto &i : string)
    {
        if (i == 'XORSTR("')
        {
            log.stream << 'XORSTR("';
        }
        log.stream << i;
    }
    log.stream << 'XORSTR("';
    log.columns++;
    return log;
}

csv_stream &operator<<(csv_stream &log, const csv_stream::end_t &end)
{
    if (!log.stream.good())
    {
        logging::Info(XORSTR("[ERROR] csvstream is not open!"));
        if (!log.open())
            return log;
    }
    log.stream << '\n';
    log.stream.flush();
    log.columns = 0;
    return log;
}

csv_stream &logger()
{
    static csv_stream object{};
    return object;
}

void LogMessage(int eid, std::string message)
{
    if (!enabled)
    {
        return;
    }
    if (dont_log_spam && hacks::shared::spam::spam_source and
        eid == g_IEngine->GetLocalPlayer())
        return;
    player_info_s info;
    if (not g_IEngine->GetPlayerInfo(eid, &info))
        return;
    if (dont_log_ipc &&
        playerlist::AccessData(info.friendsID).state ==
            playerlist::k_EState::IPC)
        return;

    std::string name(info.name);
    for (auto &x : name)
    {
        if (x == '\n' || x == '\r')
            x = '*';
    }
    for (auto &x : message)
    {
        if (x == '\n' || x == '\r')
            x = '*';
    }
    logger() << std::to_string(time(nullptr)) << std::to_string(info.friendsID)
             << name << message << std::to_string(ipc::peer->client_id)
             << csv_stream::end;
}
}
