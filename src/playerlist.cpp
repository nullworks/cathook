/*
 * playerlist.cpp
 *
 *  Created on: Apr 11, 2017
 *      Author: nullifiedcat
 */

#include "playerlist.hpp"
#include "common.h"

#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>

namespace playerlist {

std::unordered_map<unsigned, userdata> data {};

constexpr userdata null_data {};

bool ShouldSave(const userdata& data) {
	return data.color || (data.state != k_EState::DEFAULT);
}

void Save() {
	uid_t uid = geteuid();
	passwd* pw = getpwuid(uid);
	if (!pw) {
		logging::Info("Couldn't get username!");
		return;
	}
	std::string name(pw->pw_name);
	DIR* cathook_directory = opendir(strfmt("/home/%s/.cathook", pw->pw_name));
	if (!cathook_directory) {
		logging::Info(".cathook directory doesn't exist, creating one!");
		mkdir(strfmt("/home/%s/.cathook", pw->pw_name), S_IRWXU | S_IRWXG);
	} else closedir(cathook_directory);
	try {
		std::ofstream file("/home/" + name + "/.cathook/plist", std::ios::out | std::ios::binary);
		file.write(reinterpret_cast<const char*>(&SERIALIZE_VERSION), sizeof(SERIALIZE_VERSION));
		int size = 0;
		for (const auto& item : data) {
			if (ShouldSave(item.second)) size++;
		}
		file.write(reinterpret_cast<const char*>(&size), sizeof(size));
		for (const auto& item : data) {
			if (!ShouldSave(item.second)) continue;
			file.write(reinterpret_cast<const char*>(&item.first), sizeof(item.first));
			file.write(reinterpret_cast<const char*>(&item.second), sizeof(item.second));
		}
		file.close();
		logging::Info("Writing successful");
	} catch (std::exception& e) {
		logging::Info("Writing unsuccessful: %s", e.what());
	}
}

void Load() {
	data.clear();
	uid_t uid = geteuid();
	passwd* pw = getpwuid(uid);
	if (!pw) {
		logging::Info("Couldn't get username!");
		return;
	}
	std::string name(pw->pw_name);
	DIR* cathook_directory = opendir(strfmt("/home/%s/.cathook", pw->pw_name));
	if (!cathook_directory) {
		logging::Info(".cathook directory doesn't exist, creating one!");
		mkdir(strfmt("/home/%s/.cathook", pw->pw_name), S_IRWXU | S_IRWXG);
	} else closedir(cathook_directory);
	try {
		std::ifstream file("/home/" + name + "/.cathook/plist", std::ios::in | std::ios::binary);
		int file_serialize = 0;
		file.read(reinterpret_cast<char*>(&file_serialize), sizeof(file_serialize));
		if (file_serialize != SERIALIZE_VERSION) {
			logging::Info("Outdated/corrupted playerlist file! Cannot load this.");
			file.close();
			return;
		}
		int count = 0;
		file.read(reinterpret_cast<char*>(&count), sizeof(count));
		logging::Info("Reading %i entries...", count);
		for (int i = 0; i < count; i++) {
			int steamid;
			userdata udata;
			file.read(reinterpret_cast<char*>(&steamid), sizeof(steamid));
			file.read(reinterpret_cast<char*>(&udata), sizeof(udata));
			data.emplace(steamid, udata);
		}
		file.close();
		logging::Info("Reading successful!");
	} catch (std::exception& e) {
		logging::Info("Reading unsuccessful: %s", e.what());
	}
}

void DoNotKillMe() {
	constexpr unsigned developer_alts[] = { 306902159, 347272825, 401679596, 416491033, 289921064, 175278337 };
	for (int i = 0; i < sizeof(developer_alts) / sizeof(int); i++) AccessData(developer_alts[i]).state = k_EState::DEVELOPER;
}

int Color(unsigned steamid) {
	if (AccessData(steamid).state == k_EState::DEVELOPER) return colors::RainbowCurrent();
	if (AccessData(steamid).color) {
		return AccessData(steamid).color;
	} else {
		return k_Colors[static_cast<int>(AccessData(steamid).state)];
	}
}

int Color(CachedEntity* player) {
	if (CE_GOOD(player))
		return Color(player->player_info.friendsID);
	return 0;
}

userdata& AccessData(unsigned steamid) {
	try {
		return data.at(steamid);
	} catch (std::out_of_range& oor) {
		data.emplace(steamid, userdata{});
		return data.at(steamid);
	}
}

// Assume player is non-null
userdata& AccessData(CachedEntity* player) {
	if (CE_GOOD(player))
		return AccessData(player->player_info.friendsID);
	return AccessData(0U);
}

bool IsDefault(unsigned steamid) {
	const userdata& data = AccessData(steamid);
	return data.state == k_EState::DEFAULT && !data.color;
}

bool IsDefault(CachedEntity* entity) {
	if (CE_GOOD(entity))
		return IsDefault(entity->player_info.friendsID);
	return true;
}

CatCommand pl_save("pl_save", "Save playerlist", Save);
CatCommand pl_load("pl_load", "Load playerlist", Load);

CatCommand pl_set_state("pl_set_state", "pl_set_state uniqueid state\nfor example pl_set_state 306902159 0", [](const CCommand& args) {
	if (args.ArgC() < 3) {
		logging::Info("Invalid call");
		return;
	}
	unsigned steamid = strtoul(args.Arg(1), nullptr, 10);
	k_EState state = static_cast<k_EState>(strtol(args.Arg(2), nullptr, 10));
	if (state < k_EState::DEFAULT || state > k_EState::STATE_LAST) state = k_EState::DEFAULT;
	AccessData(steamid).state = state;
	logging::Info("Set %d to %d", steamid, state);
});

CatCommand pl_set_color("pl_set_color", "pl_set_color uniqueid r g b", [](const CCommand& args) {
	if (args.ArgC() < 5) {
		logging::Info("Invalid call");
		return;
	}
	unsigned steamid = strtoul(args.Arg(1), nullptr, 10);
	int r = strtol(args.Arg(2), nullptr, 10);
	int g = strtol(args.Arg(3), nullptr, 10);
	int b = strtol(args.Arg(4), nullptr, 10);
	int color = colors::Create(r, g, b, 255);
	AccessData(steamid).color = color;
	logging::Info("Set %d's color to 0x%08x", steamid, (unsigned int)color);
});

CatCommand pl_info("pl_info", "pl_info uniqueid", [](const CCommand& args) {
	if (args.ArgC() < 2) {
		logging::Info("Invalid call");
		return;
	}
	unsigned steamid = strtoul(args.Arg(1), nullptr, 10);
	logging::Info("Data for %i: ", steamid);
	logging::Info("   State: %i", AccessData(steamid).state);
	int clr = AccessData(steamid).color;
	if (clr) {
		ConColorMsg(*reinterpret_cast<::Color*>(&clr), "[CUSTOM COLOR]\n");
	}
});

}
