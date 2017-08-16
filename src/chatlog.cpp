/*
 * chatlog.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: nullifiedcat
 */

#include "common.h"
#include "init.hpp"

#include <pwd.h>
#include <unistd.h>

namespace chatlog {

CatVar enabled(CV_SWITCH, "chat_log", "0", "Chat log", "Log chat to file");
CatVar message_template(CV_STRING, "chat_log_template", "[%t] [U:1:%u] %n: %m", "Log template", "%u - SteamID\n%n - name\n%m - message\n%t - time");
CatVar dont_log_spam(CV_SWITCH, "chat_log_nospam", "1", "No Spam", "Don't log your messages if spam is active");
CatVar dont_log_ipc(CV_SWITCH, "chat_log_noipc", "1", "No IPC", "Don't log messages sent by bots");

class RAIILog {
public:
	RAIILog() {
		open();
	}
	~RAIILog() {
		stream.close();
	}
	void open() {
		logging::Info("Trying to open log file");
		uid_t uid = geteuid();
		struct passwd *pw = getpwuid(uid);
		std::string uname = "";
		if (pw) {
			uname = std::string(pw->pw_name);
		}
		stream.open(DATA_PATH "/chat-" + uname + ".log", std::ios::out | std::ios::app);
	}
	void log(const std::string& msg) {
		if (stream.bad() or not stream.is_open()) {
			logging::Info("[ERROR] RAIILog stream is bad!");
			open();
			return;
		}
		stream << msg << "\n";
		stream.flush();
	}
	std::ofstream stream;
};

RAIILog& logger() {
	static RAIILog object {};
	return object;
}

void LogMessage(int eid, std::string message) {
	if (!enabled) {
		return;
	}
	if (dont_log_spam && hacks::shared::spam::spam_source and eid == g_IEngine->GetLocalPlayer())
		return;
	player_info_s info;
	if (not g_IEngine->GetPlayerInfo(eid, &info))
		return;
	if (dont_log_ipc && playerlist::AccessData(info.friendsID).state == playerlist::k_EState::IPC)
		return;
	std::string name(info.name);
	for (auto& x : name) {
		if (x == '\n' || x == '\r') x = '*';
	}
	for (auto& x : message) {
		if (x == '\n' || x == '\r') x = '*';
	}
	time_t current_time;
	struct tm * time_info;
	char timeString[9];
	time(&current_time);
	time_info = localtime(&current_time);
	strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);
	std::string msg(message_template.GetString());
	ReplaceString(msg, "%t", std::string(timeString));
	ReplaceString(msg, "%u", std::to_string(info.friendsID));
	ReplaceString(msg, "%n", name);
	ReplaceString(msg, "%m", message);
	logger().log(msg);
}

}
