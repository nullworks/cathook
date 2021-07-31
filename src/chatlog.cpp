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
#include <settings/Bool.hpp>

namespace chatlog
{
static settings::Boolean enable{ "chat-log.enable", "false" };
static settings::Boolean no_spam{ "chat-log.no-spam", "true" };
static settings::Boolean no_ipc{ "chat-log.no-ipc", "true" };

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
        logging::Info("csvstream: Trying to open log file");
        uid_t uid         = geteuid();
        struct passwd *pw = getpwuid(uid);
        std::string uname = "";
        if (pw)
        {
            uname = std::string(pw->pw_name);
        }
        else
            uname = "unknown";
        stream.open(paths::getDataPath("/chat-" + uname + ".csv"), std::ios::out | std::ios::app);
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
        logging::Info("[ERROR] csvstream is not open!");
        if (!log.open())
            return log;
    }
    if (log.columns)
        log.stream << ',';
    log.stream << '"';
    for (const auto &i : string)
    {
        if (i == '"')
        {
            log.stream << '"';
        }
        log.stream << i;
    }
    log.stream << '"';
    log.columns++;
    return log;
}

csv_stream &operator<<(csv_stream &log, const csv_stream::end_t &end)
{
    if (!log.stream.good())
    {
        logging::Info("[ERROR] csvstream is not open!");
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
    if (!enable)
    {
        return;
    }
    if (no_spam && hacks::shared::spam::isActive() and eid == g_IEngine->GetLocalPlayer())
        return;
    player_info_s info{};
    if (not GetPlayerInfo(eid, &info))
        return;
    if (no_ipc && (playerlist::AccessData(info.friendsID).state == playerlist::k_EState::IPC || playerlist::AccessData(info.friendsID).state == playerlist::k_EState::CAT))
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
    logger() << std::to_string(time(nullptr)) << std::to_string(info.friendsID) << name << message
#if ENABLE_IPC
             << std::to_string(ipc::peer ? ipc::peer->client_id : 0)
#endif
             << csv_stream::end;
}
} // namespace chatlog
