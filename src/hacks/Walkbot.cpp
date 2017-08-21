/*
 * Walkbot.cpp
 *
 *  Created on: Jul 23, 2017
 *      Author: nullifiedcat
 */

#include "../common.h"
#include "../hack.h"

#include <sys/dir.h>
#include <sys/stat.h>

namespace hacks { namespace shared { namespace walkbot {

using index_t = unsigned;
using connection = uint8_t;

constexpr unsigned VERSION = 2;
constexpr index_t BAD_NODE = unsigned(-1);
constexpr connection MAX_CONNECTIONS = 6;
constexpr connection BAD_CONNECTION = uint8_t(-1);

index_t CreateNode(const Vector& xyz);
void DeleteNode(index_t node);
float distance_2d(Vector& xyz);
void Save(std::string filename);
bool Load(std::string filename);

enum ENodeFlags {
	NF_GOOD = (1 << 0),
	NF_DUCK = (1 << 1),
	NF_JUMP = (1 << 2)
};

enum EWalkbotState {
	WB_DISABLED,
	WB_RECORDING,
	WB_EDITING,
	WB_REPLAYING
};

struct walkbot_header_s {
	unsigned version { VERSION };
	size_t node_count { 0 };
	size_t map_length { 0 };
	size_t author_length { 0 };
};

enum EConnectionFlags {
	CF_GOOD = (1 << 0),
	CF_LOW_HEALTH = (1 << 1),
	CF_LOW_AMMO = (1 << 2),

	CF_RED = (1 << 3),
	CF_BLU = (1 << 4),

	CF_CAPPED_1 = (1 << 5),
	CF_CAPPED_2 = (1 << 6),
	CF_CAPPED_3 = (1 << 7),
	CF_CAPPED_4 = (1 << 8),
	CF_CAPPED_5 = (1 << 9)
};

struct connection_s {
	index_t node { BAD_NODE };
	unsigned flags { 0 };

	void link(index_t a) {
		flags |= CF_GOOD;
		node = a;
	}
	void unlink() {
		flags = 0;
		node = BAD_NODE;
	}
	bool good() const {
		return (flags & CF_GOOD);
	}
	bool free() const {
		return not good();
	}
};

struct walkbot_node_s {
	float x { 0 };
	float y { 0 };
	float z { 0 };
	unsigned flags { 0 };
	connection_s connections[MAX_CONNECTIONS];

	Vector& xyz() {
		return *reinterpret_cast<Vector*>(&x);
	}

	connection free_connection() const {
		for (connection i = 0; i < MAX_CONNECTIONS; i++) {
			if (connections[i].free())
				return i;
		}
		return BAD_CONNECTION;
	}

	void link(index_t node) {
		connection free = free_connection();
		if (free == BAD_CONNECTION) {
			logging::Info("[wb] Too many connections! Node at (%.2f %.2f %.2f)", x, y, z);
			return;
		}
		connections[free].link(node);
	}

	void unlink(index_t node) {
		for (connection i = 0; i < MAX_CONNECTIONS; i++) {
			if (connections[i].good() and connections[i].node == node) {
				connections[i].unlink();
			}
		}
	}
}; // 40

float distance_2d(Vector& xyz) {
	float dx = xyz.x - g_pLocalPlayer->v_Origin.x;
	float dy = xyz.y - g_pLocalPlayer->v_Origin.y;
	return sqrt(dx * dx + dy * dy);
}

namespace state {

index_t free_node();

// A vector containing all loaded nodes, used in both recording and replaying
std::vector<walkbot_node_s> nodes {};

bool node_good(index_t node) {
	return node != BAD_NODE && node < nodes.size() && (nodes[node].flags & NF_GOOD);
}

// Target node when replaying, selected node when editing, last node when recording
index_t active_node { BAD_NODE };
walkbot_node_s *active() {
	if (node_good(active_node))
		return &nodes[active_node];
	return nullptr;
}

// Last reached node when replaying
index_t last_node { BAD_NODE };
walkbot_node_s *last() {
	if (node_good(last_node))
		return &nodes[last_node];
	return nullptr;
}

// Node closest to your crosshair when editing
index_t closest_node { BAD_NODE };
walkbot_node_s *closest() {
	if (node_good(closest_node))
		return &nodes[closest_node];
	return nullptr;
}

// Global state
EWalkbotState state { WB_DISABLED };

// g_pUserCmd->buttons state when last node was recorded
int last_node_buttons { 0 };

// Set to true when bot is moving to nearest node after dying/losing its active node
bool recovery { true };

// Time when bot started to move towards next point
std::chrono::system_clock::time_point time {};

// A little bit too expensive function, finds next free node or creates one if no free slots exist
index_t free_node() {
	for (index_t i = 0; i < nodes.size(); i++) {
		if (not (nodes[i].flags & NF_GOOD))
			return i;
	}

	nodes.emplace_back();
	return nodes.size() - 1;
}

}

using state::nodes;
using state::node_good;

bool HasLowAmmo() {
	// 0x13D = CBaseCombatWeapon::HasPrimaryAmmo()
	// 190 = IsBaseCombatWeapon
	// 1C1 = C_TFWeaponBase::UsesPrimaryAmmo()
	int *weapon_list = (int*)((unsigned)(RAW_ENT(LOCAL_E)) + netvar.hMyWeapons);
	for (int i = 0; weapon_list[i]; i++) {
		int handle = weapon_list[i];
		int eid = handle & 0xFFF;
		if (eid >= 32 && eid <= HIGHEST_ENTITY) {
			IClientEntity* weapon = g_IEntityList->GetClientEntity(eid);
			if (weapon and vfunc<bool(*)(IClientEntity*)>(weapon, 190, 0)(weapon) and
					   vfunc<bool(*)(IClientEntity*)>(weapon, 0x1C1, 0)(weapon) and
				   not vfunc<bool(*)(IClientEntity*)>(weapon, 0x13D, 0)(weapon)) {
				return true;
			}
		}
	}
	return false;
}

bool HasLowHealth() {
	return float(LOCAL_E->m_iHealth) / float(LOCAL_E->m_iMaxHealth) < 0.45;
}

void DeleteNode(index_t node) {
	if (not node_good(node))
		return;
	logging::Info("[wb] Deleting node %u", node);
	auto& n = nodes[node];
	for (size_t i = 0; i < MAX_CONNECTIONS; i++) {
		if (n.connections[i].good() and node_good(n.connections[i].node)) {
			nodes[n.connections[i].node].unlink(node);
		}
	}
	memset(&n, 0, sizeof(walkbot_node_s));
}

#define BINARY_FILE_WRITE(handle, data) handle.write(reinterpret_cast<const char*>(&data), sizeof(data))
#define BINARY_FILE_READ(handle, data) handle.read(reinterpret_cast<char*>(&data), sizeof(data))

void Save(std::string filename) {
	if (g_Settings.bInvalid) {
		logging::Info("Not in-game, cannot save!");
		return;
	}
	{
		DIR* walkbot_dir = opendir(DATA_PATH "/walkbot");
		if (!walkbot_dir) {
			logging::Info("Walkbot directory doesn't exist, creating one!");
			mkdir(DATA_PATH "/walkbot", S_IRWXU | S_IRWXG);
		} else closedir(walkbot_dir);
	}
	std::string path = format(DATA_PATH "/walkbot/", GetLevelName());
	{
		DIR* level_dir = opendir(path.c_str());
		if (!level_dir) {
			logging::Info("Walkbot directory for %s doesn't exist, creating one!", GetLevelName().c_str());
			mkdir(path.c_str(), S_IRWXU | S_IRWXG);
		} else closedir(level_dir);
	}
	logging::Info("Saving in %s", format(path, "/", filename).c_str());
	try {
		std::ofstream file(format(path, "/", filename), std::ios::out | std::ios::binary);
		if (not file) {
			logging::Info("Could not open file!");
			return;
		}
		walkbot_header_s header;
		header.node_count = state::nodes.size();
		const char* name = g_ISteamFriends->GetPersonaName();
		const char* map  = g_IEngine->GetLevelName();
		size_t name_s = strlen(name);
		size_t map_s  = strlen(map);
		header.author_length = name_s;
		header.map_length = map_s;
		BINARY_FILE_WRITE(file, header);
		file.write(map, map_s);
		file.write(name, name_s);
		file.write(reinterpret_cast<const char*>(state::nodes.data()), sizeof(walkbot_node_s) * header.node_count);
		file.close();
		logging::Info("Writing successful");
	} catch (std::exception& e) {
		logging::Info("Writing unsuccessful: %s", e.what());
	}
}

bool Load(std::string filename) {
	{
		DIR* walkbot_dir = opendir(DATA_PATH "/walkbot");
		if (!walkbot_dir) {
			logging::Info("Walkbot directory doesn't exist!");
			return false;
		} else closedir(walkbot_dir);
	}
	std::string path = format(DATA_PATH "/walkbot/", GetLevelName());
	{
		DIR* level_dir = opendir(path.c_str());
		if (!level_dir) {
			logging::Info("Walkbot directory for %s doesn't exist, creating one!", GetLevelName().c_str());
			mkdir(path.c_str(), S_IRWXU | S_IRWXG);
		} else closedir(level_dir);
	}
	try {
		std::ifstream file(format(path, "/", filename), std::ios::in | std::ios::binary);
		if (!file) {
			return false;
		}
		walkbot_header_s header;
		BINARY_FILE_READ(file, header);
		// FIXME magic number: 1
		if (header.version != VERSION) {
			logging::Info("Outdated/corrupted walkbot file! Cannot load this.");
			file.close();
			return false;
		}
		if (header.author_length > 64 or header.map_length > 512 or (not header.author_length or not header.map_length)) {
			logging::Info("Corrupted author/level data");
		} else {
			char name_buffer[header.author_length + 1];
			char map_buffer[header.map_length + 1];
			file.read(map_buffer, header.map_length);
			file.read(name_buffer, header.author_length);
			name_buffer[header.author_length] = 0;
			map_buffer[header.map_length] = 0;
			logging::Info("Walkbot navigation map for %s\nAuthor: %s", map_buffer, name_buffer);
		}
		state::nodes.clear();
		logging::Info("Reading %i entries...", header.node_count);
		if (header.node_count > 32768) {
			logging::Info("Read %d nodes, max is %d. Aborting.", header.node_count, 32768);
			return false;
		}
		state::nodes.resize(header.node_count);
		file.read(reinterpret_cast<char*>(state::nodes.data()), sizeof(walkbot_node_s) * header.node_count);
		file.close();
		logging::Info("Reading successful! Result: %i entries.", state::nodes.size());
		return true;
	} catch (std::exception& e) {
		logging::Info("Reading unsuccessful: %s", e.what());
	}
	return false;
}

static CatCommand save("wb_save", "Save", [](const CCommand& args) {
	logging::Info("Saving");
	std::string filename = "default";
	if (args.ArgC() > 1) {
		filename = args.Arg(1);
	}
	Save(filename);
});
static CatCommand load("wb_load", "Load", [](const CCommand& args) {
	logging::Info("Loading");
	std::string filename = "default";
	if (args.ArgC() > 1) {
		filename = args.Arg(1);
	}
	Load(filename);
});

index_t CreateNode(const Vector& xyz) {
	index_t node = state::free_node();
	logging::Info("[wb] Creating node %u at (%.2f %.2f %.2f)", node, xyz.x, xyz.y, xyz.z);
	auto& n = state::nodes[node];
	memset(&n, 0, sizeof(n));
	n.xyz() = xyz;
	n.flags |= NF_GOOD;
	return node;
}

CatVar active_recording(CV_SWITCH, "wb_recording", "0", "Do recording", "Use BindToggle with this");
CatVar draw_info(CV_SWITCH, "wb_info", "1", "Walkbot info");
CatVar draw_path(CV_SWITCH, "wb_path", "1", "Walkbot path");
CatVar draw_nodes(CV_SWITCH, "wb_nodes", "1", "Walkbot nodes");
CatVar draw_indices(CV_SWITCH, "wb_indices", "0", "Node indices");
CatVar free_move(CV_SWITCH, "wb_freemove", "1", "Allow free movement", "Allow free movement while pressing movement keys");
CatVar spawn_distance(CV_FLOAT, "wb_node_spawn_distance", "54", "Node spawn distance");
CatVar max_distance(CV_FLOAT, "wb_replay_max_distance", "100", "Max distance to node when replaying");
CatVar reach_distance(CV_FLOAT, "wb_replay_reach_distance", "32", "Distance where bot can be considered 'stepping' on the node");
CatVar draw_connection_flags(CV_SWITCH, "wb_connection_flags", "1", "Connection flags");
CatVar force_slot(CV_INT, "wb_force_slot", "1", "Force slot", "Walkbot will always select weapon in this slot");
CatVar leave_if_empty(CV_SWITCH, "wb_leave_if_empty", "0", "Leave if no walkbot", "Leave game if there is no walkbot map");

CatCommand c_start_recording("wb_record", "Start recording", []() { state::state = WB_RECORDING; });
CatCommand c_start_editing("wb_edit", "Start editing", []() { state::state = WB_EDITING; });
CatCommand c_start_replaying("wb_replay", "Start replaying", []() {
	state::last_node = state::active_node;
	state::active_node = state::closest_node;
	state::state = WB_REPLAYING;
});
CatCommand c_exit("wb_exit", "Exit", []() { state::state = WB_DISABLED; });

// Selects closest node, clears selection if node is selected
CatCommand c_select_node("wb_select", "Select node", []() {
	if (state::active_node == state::closest_node) {
		state::active_node = BAD_NODE;
	} else {
		state::active_node = state::closest_node;
	}
});
// Makes a new node in the middle of connection between 2 nodes
CatCommand c_split_connection("wb_split", "Split connection", []() {
	if (not (state::node_good(state::active_node) and state::node_good(state::closest_node)))
		return;

	if (state::active_node == state::closest_node)
		return;

	auto& a = state::nodes[state::active_node];
	auto& b = state::nodes[state::closest_node];

	a.unlink(state::closest_node);
	b.unlink(state::active_node);

	index_t node = CreateNode((a.xyz() + b.xyz()) / 2);
	auto& n = state::nodes[node];
	a.link(node);
	n.link(state::active_node);
	b.link(node);
	n.link(state::closest_node);

});
// Deletes closest node and its connections
CatCommand c_delete_node("wb_delete", "Delete node", []() {
	DeleteNode(state::closest_node);
});
// Creates a new node under your feet and connects it to closest node to your crosshair
CatCommand c_create_node("wb_create", "Create node", []() {
	index_t node = CreateNode(g_pLocalPlayer->v_Origin);
	auto& n = state::nodes[node];
	if (g_pUserCmd->buttons & IN_DUCK)
		n.flags |= NF_DUCK;
	if (state::node_good(state::closest_node)) {
		auto& c = state::nodes[state::closest_node];
		n.link(state::closest_node);
		c.link(node);
		logging::Info("[wb] Node %u linked to node %u at (%.2f %.2f %.2f)", node, state::closest_node, c.x, c.y, c.z);
	}
});
// Connects selected node to closest one
CatCommand c_connect_node("wb_connect", "Connect nodes", []() {
	if (not (state::node_good(state::active_node) and state::node_good(state::closest_node)))
		return;
	// Don't link a node to itself, idiot
	if (state::active_node == state::closest_node)
		return;

	auto& a = state::nodes[state::active_node];
	auto& b = state::nodes[state::closest_node];

	a.link(state::closest_node);
	b.link(state::active_node);
});
// Makes a one-way connection
CatCommand c_connect_single_node("wb_connect_single", "Connect nodes (one-way)", []() {
	if (not (state::node_good(state::active_node) and state::node_good(state::closest_node)))
		return;
	// Don't link a node to itself, idiot
	if (state::active_node == state::closest_node)
		return;

	auto& a = state::nodes[state::active_node];

	a.link(state::closest_node);
});
// Connects selected node to closest one
CatCommand c_disconnect_node("wb_disconnect", "Disconnect nodes", []() {
	if (not (state::node_good(state::active_node) and state::node_good(state::closest_node)))
		return;
	// Don't link a node to itself, idiot
	if (state::active_node == state::closest_node)
		return;

	auto& a = state::nodes[state::active_node];
	auto& b = state::nodes[state::closest_node];

	a.unlink(state::closest_node);
	b.unlink(state::active_node);
});
// Makes a one-way connection
CatCommand c_disconnect_single_node("wb_disconnect_single", "Connect nodes (one-way)", []() {
	if (not (state::node_good(state::active_node) and state::node_good(state::closest_node)))
		return;
	// Don't link a node to itself, idiot
	if (state::active_node == state::closest_node)
		return;

	auto& a = state::nodes[state::active_node];

	a.unlink(state::closest_node);
});
// Toggles jump flag on closest node
CatCommand c_update_duck("wb_duck", "Toggle duck flag", []() {
	if (not state::node_good(state::closest_node))
		return;

	auto& n = state::nodes[state::closest_node];

	if (n.flags & NF_DUCK)
		n.flags &= ~NF_DUCK;
	else
		n.flags |= NF_DUCK;
});
// Toggles jump flag on closest node
CatCommand c_update_jump("wb_jump", "Toggle jump flag", []() {
	if (not state::node_good(state::closest_node))
		return;

	auto& n = state::nodes[state::closest_node];

	if (n.flags & NF_JUMP)
		n.flags &= ~NF_JUMP;
	else
		n.flags |= NF_JUMP;
});
// Assuming node is good and conn is in range [0; MAX_CONNECTIONS)
std::string DescribeConnection(index_t node, connection conn) {
	const connection_s& c = nodes[node].connections[conn];
	std::string extra;
	bool broken = not node_good(c.node);
	bool oneway = true;
	if (not broken) {
		auto& n = state::nodes[c.node];
		for (size_t j = 0; j < MAX_CONNECTIONS; j++) {
			if (n.connections[j].good() and n.connections[j].node == node) {
				oneway = false;
				break;
			}
		}
		if (c.flags & CF_LOW_AMMO) {
			extra += "A";
		}
		if (c.flags & CF_LOW_HEALTH) {
			extra += "H";
		}
	}
	std::string result = format(node, ' ', (broken ? "-x>" : (oneway ? "-->" : "<->")), ' ', c.node, ' ', extra);
	return result;
}
CatCommand c_toggle_cf_ammo("wb_conn_ammo", "Toggle 'ammo' flag on connection from ACTIVE to CLOSEST node", []() {
	auto a = state::active();
	auto b = state::closest();
	if (not (a and b)) return;
	for (connection i = 0; i < MAX_CONNECTIONS; i++) {
		auto& c = a->connections[i];
		if (c.free())
			continue;
		if (c.node != state::closest_node)
			continue;
		// Actually flip the flag
		if (c.flags & CF_LOW_AMMO)
			c.flags &= ~CF_LOW_AMMO;
		else
			c.flags |= CF_LOW_AMMO;
	}
});
CatCommand c_toggle_cf_health("wb_conn_health", "Toggle 'health' flag on connection from ACTIVE to CLOSEST node", []() {
	auto a = state::active();
	auto b = state::closest();
	if (not (a and b)) return;
	for (connection i = 0; i < MAX_CONNECTIONS; i++) {
		auto& c = a->connections[i];
		if (c.free())
			continue;
		if (c.node != state::closest_node)
			continue;
		// Actually flip the flag
		if (c.flags & CF_LOW_HEALTH)
			c.flags &= ~CF_LOW_HEALTH;
		else
			c.flags |= CF_LOW_HEALTH;
	}
});
// Displays all info about closest node and its connections
CatCommand c_info("wb_dump", "Show info", []() {
	index_t node = state::closest_node;
	if (not node_good(node))
		return;

	auto& n = nodes[node];

	logging::Info("[wb] Info about node %u", node);
	logging::Info("[wb] Flags: Duck=%d, Jump=%d, Raw=%u", n.flags & NF_DUCK, n.flags & NF_JUMP, n.flags);
	logging::Info("[wb] X: %.2f | Y: %.2f | Z: %.2f", n.x, n.y, n.z);
	logging::Info("[wb] Connections:");
	for (size_t i = 0; i < MAX_CONNECTIONS; i++) {
		if (n.connections[i].free())
			continue;
		logging::Info("[wb] %s", DescribeConnection(node, i).c_str());
	}
});
// Deletes a whole region of nodes
// Deletes a single closest node if no node is selected
CatCommand c_delete_region("wb_delete_region", "Delete region of nodes", []() {
	logging::Info("< DISABLED >");
	/*index_t a = state::active_node;
	index_t b = state::closest_node;

	if (not (state::node_good(a) and state::node_good(b)))
		return;

	index_t current = state::closest_node;
	index_t next = INVALID_NODE;

	do {
		auto& n = state::nodes[current];

		if (n.connection_count > 2) {
			logging::Info("[wb] More than 2 connections on a node! Quitting.");
			return;
		}
		bool found_next = false;
		for (size_t i = 0; i < 2; i++) {
			if (n.connections[i] != current) {
				next = n.connections[i];
				found_next = true;
			}
		}
		DeleteNode(current);
		current = next;
		if (not found_next) {
			logging::Info("[wb] Dead end? Can't find next node after %u", current);
			break;
		}
	} while (state::node_good(current) and (current != a));*/
});
// Clears the state
CatCommand c_clear("wb_clear", "Removes all nodes", []() {
	state::nodes.clear();
});

void Initialize() {
}

void UpdateClosestNode() {
	float n_fov = 360.0f;
	index_t n_idx = BAD_NODE;

	for (index_t i = 0; i < state::nodes.size(); i++) {
		auto& node = state::nodes[i];

		if (not node.flags & NF_GOOD)
			continue;
		// Eclipse shits itself when it sees Vector& beung used as Vector in GetFov
		float fov = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, node.xyz());
		if (fov < n_fov) {
			n_fov = fov;
			n_idx = i;
		}
	}

	// Don't select a node if you don't even look at it
	if (n_fov < 10)
		state::closest_node = n_idx;
	else
		state::closest_node = BAD_NODE;
}

// Finds nearest node by position, not FOV
// Not to be confused with FindClosestNode
index_t FindNearestNode(bool traceray) {
	index_t r_node { BAD_NODE };
	float r_dist { 65536.0f };

	for (index_t i = 0; i < state::nodes.size(); i++) {
		if (state::node_good(i)) {
			auto& n = state::nodes[i];
			if (traceray and not IsVectorVisible(g_pLocalPlayer->v_Eye, n.xyz()))
				continue;
			float dist = distance_2d(n.xyz());
			if (dist < r_dist) {
				r_dist = dist;
				r_node = i;
			}
		}
	}

	return r_node;
}

index_t SelectNextNode() {
	if (not state::node_good(state::active_node)) {
		return FindNearestNode(true);
	}
	auto& n = state::nodes[state::active_node];
	// TODO medkit connections and shit
	std::vector<index_t> chance {};
	for (index_t i = 0; i < MAX_CONNECTIONS; i++) {
		if (n.connections[i].good() and n.connections[i].node != state::last_node and node_good(n.connections[i].node)) {
			if (HasLowAmmo() && (n.connections[i].flags & CF_LOW_AMMO)) {
				return n.connections[i].node;
			}
			if (HasLowHealth() && (n.connections[i].flags & CF_LOW_HEALTH)) {
				return n.connections[i].node;
			}
			if (not (n.connections[i].flags & (CF_LOW_AMMO | CF_LOW_HEALTH)))
				chance.push_back(n.connections[i].node);
		}
	}
	if (not chance.empty()) {
		return chance.at(unsigned(rand()) % chance.size());
	} else {
		return BAD_NODE;
	}
	return BAD_NODE;
}

bool free_move_used = false;

void UpdateSlot() {
	static auto last_check = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_check).count();

	if (CE_GOOD(LOCAL_E) && CE_GOOD(LOCAL_W) && !g_pLocalPlayer->life_state && ms > 1000) {
		IClientEntity* weapon = RAW_ENT(LOCAL_W);
		// IsBaseCombatWeapon()
		if (vfunc<bool(*)(IClientEntity*)>(weapon, 190, 0)(weapon)) {
			int slot = vfunc<int(*)(IClientEntity*)>(weapon, 395, 0)(weapon);
			if (slot != int(force_slot) - 1) {
				hack::ExecuteCommand(format("slot", int(force_slot)));
			}
		}
	}

	last_check = std::chrono::system_clock::now();
}

void UpdateWalker() {
	free_move_used = false;
	if (free_move) {
		if (g_pUserCmd->forwardmove != 0.0f or g_pUserCmd->sidemove != 0.0f) {
			free_move_used = true;
			return;
		}
	}

	static int jump_ticks = 0;
	if (jump_ticks > 0) {
		g_pUserCmd->buttons |= IN_JUMP;
		jump_ticks--;
	}
	bool timeout = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - state::time).count() > 1;
	if (not state::node_good(state::active_node) or timeout) {
		state::active_node = FindNearestNode(true);
		state::recovery = true;
	}
	auto& n = state::nodes[state::active_node];
	WalkTo(n.xyz());
	if (state::node_good(state::last_node)) {
		auto& l = state::nodes[state::last_node];
		if (l.flags & NF_DUCK)
			g_pUserCmd->buttons |= IN_DUCK;
	}
	float dist = distance_2d(n.xyz());
	if (dist > float(max_distance)) {
		 state::active_node = FindNearestNode(true);
		 state::recovery = true;
	}
	if (dist < float(reach_distance)) {
		state::recovery = false;
		index_t last = state::active_node;
		state::active_node = SelectNextNode();
		state::last_node = last;
		state::time = std::chrono::system_clock::now();
		if (state::node_good(state::active_node)) {
			if (state::nodes[state::active_node].flags & NF_JUMP) {
				g_pUserCmd->buttons |= IN_DUCK;
				g_pUserCmd->buttons |= IN_JUMP;
				jump_ticks = 6;
			}
		} else {
			if (not state::recovery) {
				state::recovery = true;
			}
		}
	}
}

bool ShouldSpawnNode() {
	if (not state::node_good(state::active_node))
		return true;

	bool was_jumping = state::last_node_buttons & IN_JUMP;
	bool is_jumping = g_pUserCmd->buttons & IN_JUMP;

	if (was_jumping != is_jumping and is_jumping)
		return true;

	if ((state::last_node_buttons & IN_DUCK) != (g_pUserCmd->buttons & IN_DUCK))
		return true;

	auto& node = state::nodes[state::active_node];

	if (distance_2d(node.xyz()) > float(spawn_distance)) {
		return true;
	}

	return false;
}

void RecordNode() {
	index_t node = CreateNode(g_pLocalPlayer->v_Origin);
	auto& n = state::nodes[node];
	if (g_pUserCmd->buttons & IN_DUCK)
		n.flags |= NF_DUCK;
	if (g_pUserCmd->buttons & IN_JUMP)
		n.flags |= NF_JUMP;
	if (state::node_good(state::active_node)) {
		auto& c = state::nodes[state::active_node];
		n.link(state::active_node);
		c.link(node);
		logging::Info("[wb] Node %u auto-linked to node %u at (%.2f %.2f %.2f)", node, state::active_node, c.x, c.y, c.z);
	}
	state::last_node_buttons = g_pUserCmd->buttons;
	state::active_node = node;
}

#if ENABLE_VISUALS == 1

// Draws a single colored connection between 2 nodes
void DrawConnection(index_t a, connection_s& b) {
	if (b.free())
		return;
	if (not (node_good(a) and node_good(b.node)))
		return;

	auto& a_ = state::nodes[a];
	auto& b_ = state::nodes[b.node];

	Vector center = (a_.xyz() + b_.xyz()) / 2;
	Vector center_connection = (a_.xyz() + center) / 2;

	Vector wts_a, wts_c, wts_cc;
	if (not (draw::WorldToScreen(a_.xyz(), wts_a) and draw::WorldToScreen(center, wts_c) and draw::WorldToScreen(center_connection, wts_cc)))
		return;

	rgba_t* color = &colors::white;
	if 		((a_.flags & b_.flags) & NF_JUMP) color = &colors::yellow;
	else if ((a_.flags & b_.flags) & NF_DUCK) color = &colors::green;

	drawgl::Line(wts_a.x, wts_a.y, wts_c.x - wts_a.x, wts_c.y - wts_a.y, color->rgba);

	if (draw_connection_flags && b.flags != CF_GOOD) {
		std::string flags;
		if (b.flags & CF_LOW_AMMO) flags += "A";
		if (b.flags & CF_LOW_HEALTH) flags += "H";
		int size_x = 0, size_y = 0;
		FTGL_StringLength(flags, fonts::font_main, &size_x, &size_y);
		FTGL_Draw(flags, wts_cc.x - size_x / 2, wts_cc.y - size_y - 4, fonts::font_main);
	}
}

// Draws a node and its connections
void DrawNode(index_t node, bool draw_back) {
	if (not state::node_good(node))
		return;

	auto& n = state::nodes[node];

	for (size_t i = 0; i < MAX_CONNECTIONS; i++) {
		DrawConnection(node, n.connections[i]);
	}

	if (draw_nodes) {
		rgba_t* color = &colors::white;
		if 		(n.flags & NF_JUMP) color = &colors::yellow;
		else if (n.flags & NF_DUCK) color = &colors::green;

		Vector wts;
		if (not draw::WorldToScreen(n.xyz(), wts))
			return;

		size_t node_size = 2;
		if (state::state != WB_REPLAYING) {
			if (node == state::closest_node)
				node_size = 6;
		}
		if (node == state::active_node)
			color = &colors::red;

		drawgl::FilledRect(wts.x - node_size, wts.y - node_size, 2 * node_size, 2 * node_size, color->rgba);
	}

	if (draw_indices) {
		rgba_t* color = &colors::white;
		if 		(n.flags & NF_JUMP) color = &colors::yellow;
		else if (n.flags & NF_DUCK) color = &colors::green;

		Vector wts;
		if (not draw::WorldToScreen(n.xyz(), wts))
			return;

		FTGL_Draw(std::to_string(node), wts.x, wts.y, fonts::font_main, *color);
	}
}

void DrawPath() {
	for (index_t i = 0; i < state::nodes.size(); i++) {
		DrawNode(i, true);
	}
}

void Draw() {
	if (state::state == WB_DISABLED) return;
	switch (state::state) {
	case WB_RECORDING: {
		AddSideString("Walkbot: Recording");
	} break;
	case WB_EDITING: {
		AddSideString("Walkbot: Editing");
	} break;
	case WB_REPLAYING: {
		AddSideString("Walkbot: Replaying");
		if (free_move and free_move_used) {
			AddSideString("Walkbot: FREE MOVEMENT (User override)", colors::green);
		}
		if (HasLowAmmo()) {
			AddSideString("Walkbot: LOW AMMO", colors::yellow);
		}
		if (HasLowHealth()) {
			AddSideString("Walkbot: LOW HEALTH", colors::red);
		}
	} break;
	}
	if (draw_info) {
		AddSideString(format("Active node: ", state::active_node));
		AddSideString(format("Highlighted node: ", state::closest_node));
		AddSideString(format("Last node: ", state::last_node));
		AddSideString(format("Node count: ", state::nodes.size()));
		if (state::recovery) {
			AddSideString(format("(Recovery mode)"));
		}
	}
	if (draw_path)
		DrawPath();
}

#endif

void OnLevelInit() {
	if (leave_if_empty && state::state == WB_REPLAYING) {
		nodes.clear();
	}
}

static CatVar wb_abandon_too_many_bots(CV_INT, "wb_population_control", "0", "Abandon if bots >");
void CheckLivingSpace() {
#if ENABLE_IPC
	if (ipc::peer && wb_abandon_too_many_bots) {
		std::vector<unsigned> players {};
		for (int j = 1; j < 32; j++) {
			player_info_s info;
			if (g_IEngine->GetPlayerInfo(j, &info)) {
				if (info.friendsID)
					players.push_back(info.friendsID);
			}
		}
		int count = 0;
		unsigned highest = 0;
		std::vector<unsigned> botlist {};
		for (unsigned i = 0; i < cat_ipc::max_peers; i++) {
			if (!ipc::peer->memory->peer_data[i].free) {
				for (auto& k : players) {
					if (ipc::peer->memory->peer_user_data[i].friendid && k == ipc::peer->memory->peer_user_data[i].friendid) {
						botlist.push_back(i);
						count++;
						highest = i;
					}
				}
			}
		}
		if (ipc::peer->client_id == highest && count > int(wb_abandon_too_many_bots)) {
			static Timer timer {};
			if (timer.test_and_set(1000 * 5)) {
				logging::Info("Found %d other bots in-game, abandoning (%u)", count, ipc::peer->client_id);
				for (auto i : botlist) {
					logging::Info("-> Bot %d with ID %u", i, ipc::peer->memory->peer_user_data[i].friendid);
				}
				g_TFGCClientSystem->SendExitMatchmaking(true);
			}
		}
	}
#endif
}

void Move() {
	if (state::state == WB_DISABLED) return;
	switch (state::state) {
	case WB_RECORDING: {
		UpdateClosestNode();
		if (active_recording and ShouldSpawnNode()) {
			RecordNode();
		}
	} break;
	case WB_EDITING: {
		UpdateClosestNode();
	} break;
	case WB_REPLAYING: {
		if (leave_if_empty) {
			if (nodes.size() == 0) {
				Load("default");
				if (nodes.size() == 0) {
					static auto last_abandon = std::chrono::system_clock::from_time_t(0);
					auto s = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_abandon).count();

					if (s < 3) {
						return;
					}
					logging::Info("No map file, shutting down");
					g_TFGCClientSystem->SendExitMatchmaking(true);
					last_abandon = std::chrono::system_clock::now();
				}
			}
		}
		static Timer livingspace_timer {};
		if (livingspace_timer.test_and_set(1000 * 8)) {
			CheckLivingSpace();
		}
		if (nodes.size() == 0) return;
		if (force_slot)
			UpdateSlot();
		UpdateWalker();
	} break;
	}
}

}}}
