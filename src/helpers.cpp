/*
 * helpers.cpp
 *
 *  Created on: Oct 8, 2016
 *      Author: nullifiedcat
 */

#include "common.h"
#include "hooks.h"
#include "sdk.h"
#include "profiler.h"

#include <sys/mman.h>

std::vector<ConVar*>& RegisteredVarsList() {
	static std::vector<ConVar*> list {};
	return list;
}

std::vector<ConCommand*>& RegisteredCommandsList() {
	static std::vector<ConCommand*> list {};
	return list;
}

void BeginConVars() {
	logging::Info("Begin ConVars");
	if (!std::ifstream("tf/cfg/cat_autoexec.cfg")) {
		std::ofstream cfg_autoexec("tf/cfg/cat_autoexec.cfg", std::ios::out | std::ios::trunc);
		if (cfg_autoexec.good()) {
			cfg_autoexec << "// Put your custom cathook settings in this file\n// This script will be executed EACH TIME YOU INJECT CATHOOK\n";
		}
	}
	if (!std::ifstream("tf/cfg/cat_autoexec.cfg")) {
		std::ofstream cfg_autoexec("tf/cfg/cat_matchexec.cfg", std::ios::out | std::ios::trunc);
		if (cfg_autoexec.good()) {
			cfg_autoexec << "// Put your custom cathook settings in this file\n// This script will be executed EACH TIME YOU JOIN A MATCH\n";
		}
	}
	logging::Info(":b:");
	SetCVarInterface(g_ICvar);
}

void EndConVars() {
	logging::Info("Registering ConVars");
	RegisterCatVars();
	RegisterCatCommands();
	ConVar_Register();

	std::ofstream cfg_defaults("tf/cfg/cat_defaults.cfg", std::ios::out | std::ios::trunc);
	if (cfg_defaults.good()) {
		cfg_defaults << "// This file is auto-generated and will be overwritten each time you inject cathook\n// Do not make edits to this file\n\n// Every registered variable dump\n";
		for (const auto& i : RegisteredVarsList()) {
			cfg_defaults << i->GetName() << " \"" << i->GetDefault() << "\"\n";
		}
		cfg_defaults << "\n// Every registered command dump\n";
		for (const auto& i : RegisteredCommandsList()) {
			cfg_defaults << "// " << i->GetName() << "\n";
		}
	}
}

ConVar* CreateConVar(std::string name, std::string value, std::string help) {
	char* namec = new char[256];
	char* valuec = new char[256];
	char* helpc = new char[256];
	strncpy(namec, name.c_str(), 255);
	strncpy(valuec, value.c_str(), 255);
	strncpy(helpc, help.c_str(), 255);
	//logging::Info("Creating ConVar: %s %s %s", namec, valuec, helpc);
	ConVar* ret = new ConVar(const_cast<const char*>(namec), const_cast<const char*>(valuec), 0, const_cast<const char*>(helpc));
	g_ICvar->RegisterConCommand(ret);
	RegisteredVarsList().push_back(ret);
	return ret;
}

// Function for when you want to goto a vector
void WalkTo(const Vector& vector) {
	if (CE_BAD(LOCAL_E)) return;
	// Calculate how to get to a vector
	auto result = ComputeMove(LOCAL_E->m_vecOrigin, vector);
	// Push our move to usercmd
	g_pUserCmd->forwardmove = result.first;
	g_pUserCmd->sidemove = result.second;
}


std::string GetLevelName() {

	std::string name(g_IEngine->GetLevelName());
	size_t slash = name.find('/');
	if (slash == std::string::npos) slash = 0;
	else slash++;
	size_t bsp = name.find(".bsp");
	size_t length = (bsp == std::string::npos ? name.length() - slash : bsp - slash);
	return name.substr(slash, length);
}

std::pair<float, float> ComputeMove(const Vector& a, const Vector& b) {
	Vector diff = (b - a);
	if (diff.Length() == 0) return { 0, 0 };
	const float x = diff.x;
	const float y = diff.y;
	Vector vsilent(x, y, 0);
	float speed = sqrt(vsilent.x * vsilent.x + vsilent.y * vsilent.y);
	Vector ang;
	VectorAngles(vsilent, ang);
	float yaw = DEG2RAD(ang.y - g_pUserCmd->viewangles.y);
	return { cos(yaw) * 450, -sin(yaw) * 450 };
}

ConCommand* CreateConCommand(const char* name, FnCommandCallback_t callback, const char* help) {
	ConCommand* ret = new ConCommand(name, callback, help);
	g_ICvar->RegisterConCommand(ret);
	RegisteredCommandsList().push_back(ret);
	return ret;
}

const char* GetBuildingName(CachedEntity* ent) {
	if (!ent) return "[NULL]";
	if (ent->m_iClassID == CL_CLASS(CObjectSentrygun)) return "Sentry";
	if (ent->m_iClassID == CL_CLASS(CObjectDispenser)) return "Dispenser";
	if (ent->m_iClassID == CL_CLASS(CObjectTeleporter)) return "Teleporter";
	return "[NULL]";
}

void format_internal(std::stringstream& stream) {
	(void)(stream);
}

void ReplaceString(std::string& input, const std::string& what, const std::string& with_what) {
	size_t index;
	index = input.find(what);
	while (index != std::string::npos) {
		input.replace(index, what.size(), with_what);
		index = input.find(what, index + with_what.size());
	}
}

powerup_type GetPowerupOnPlayer(CachedEntity* player) {
	if (CE_BAD(player)) return powerup_type::not_powerup;
//	if (!HasCondition<TFCond_HasRune>(player)) return powerup_type::not_powerup;
	if (HasCondition<TFCond_RuneStrength>(player)) return powerup_type::strength;
	if (HasCondition<TFCond_RuneHaste>(player)) return powerup_type::haste;
	if (HasCondition<TFCond_RuneRegen>(player)) return powerup_type::regeneration;
	if (HasCondition<TFCond_RuneResist>(player)) return powerup_type::resistance;
	if (HasCondition<TFCond_RuneVampire>(player)) return powerup_type::vampire;
	if (HasCondition<TFCond_RuneWarlock>(player)) return powerup_type::reflect;
	if (HasCondition<TFCond_RunePrecision>(player)) return powerup_type::precision;
	if (HasCondition<TFCond_RuneAgility>(player)) return powerup_type::agility;
	if (HasCondition<TFCond_RuneKnockout>(player)) return powerup_type::knockout;
	if (HasCondition<TFCond_KingRune>(player)) return powerup_type::king;
	if (HasCondition<TFCond_PlagueRune>(player)) return powerup_type::plague;
	if (HasCondition<TFCond_SupernovaRune>(player)) return powerup_type::supernova;
	return powerup_type::not_powerup;
}

// A function to tell if a player is using a specific weapon
bool HasWeapon(CachedEntity* ent, int wantedId) {
	if (CE_BAD(ent)) return false;
	// Create a var to store the handle
	int *hWeapons;
	// Grab the handle and store it into the var
	hWeapons = (int*)((unsigned)(RAW_ENT(ent) + netvar.hMyWeapons));
	// Go through the handle array and search for the item
	for (int i = 0; hWeapons[i]; i++) {
		// Get the weapon id from the handle array
		IClientEntity* weapon = g_IEntityList->GetClientEntityFromHandle(hWeapons[i]);
		// if weapon is what we are looking for, return true
		if (weapon && NET_INT(weapon, netvar.iItemDefinitionIndex) == wantedId) return true;
	}
	// We didnt find the weapon we needed, return false
	return false;
}

bool HasDarwins(CachedEntity* ent) {
	if (CE_BAD(ent)) return false;
	// Check if player is sniper
	if (CE_INT(ent, netvar.iClass) != tf_sniper) return false;
	// Check if player is using darwins, 231 is the id for darwins danger sheild
	if (HasWeapon(ent, 231)) return true;
	// Else return false
	return false;
}

void VectorTransform (const float *in1, const matrix3x4_t& in2, float *out) {
	out[0] = (in1[0] * in2[0][0] + in1[1] * in2[0][1] + in1[2] * in2[0][2]) + in2[0][3];
	out[1] = (in1[0] * in2[1][0] + in1[1] * in2[1][1] + in1[2] * in2[1][2]) + in2[1][3];
	out[2] = (in1[0] * in2[2][0] + in1[1] * in2[2][1] + in1[2] * in2[2][2]) + in2[2][3];
}

bool GetHitbox(CachedEntity* entity, int hb, Vector& out) {
	hitbox_cache::CachedHitbox *box;

	if (CE_BAD(entity)) return false;
	box = entity->hitboxes.GetHitbox(hb);
	if (!box) out = entity->m_vecOrigin;
	else out = box->center;
	return true;
}

void VectorAngles(Vector &forward, Vector &angles) {
	float tmp, yaw, pitch;

	if(forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if(forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / PI);
		if(yaw < 0)
			yaw += 360;

		tmp = sqrt((forward[0] * forward[0] + forward[1] * forward[1]));
		pitch = (atan2(-forward[2], tmp) * 180 / PI);
		if(pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

char GetUpperChar(ButtonCode_t button) {
	switch (button) {
	case KEY_0:
		return ')';
	case KEY_1:
		return '!';
	case KEY_2:
		return '@';
	case KEY_3:
		return '#';
	case KEY_4:
		return '$';
	case KEY_5:
		return '%';
	case KEY_6:
		return '^';
	case KEY_7:
		return '&';
	case KEY_8:
		return '*';
	case KEY_9:
		return '(';
	case KEY_LBRACKET:
		return '{';
	case KEY_RBRACKET:
		return '}';
	case KEY_SEMICOLON:
		return ':';
	case KEY_BACKQUOTE:
		return '~';
	case KEY_APOSTROPHE:
		return '"';
	case KEY_COMMA:
		return '<';
	case KEY_PERIOD:
		return '>';
	case KEY_SLASH:
		return '?';
	case KEY_BACKSLASH:
		return '|';
	case KEY_MINUS:
		return '_';
	case KEY_EQUAL:
		return '+';
	default:
		if (strlen(g_IInputSystem->ButtonCodeToString(button)) != 1) return 0;
		return toupper(*g_IInputSystem->ButtonCodeToString(button));
	}
}

char GetChar(ButtonCode_t button) {
	switch (button) {
	case KEY_PAD_DIVIDE:
		return '/';
	case KEY_PAD_MULTIPLY:
		return '*';
	case KEY_PAD_MINUS:
		return '-';
	case KEY_PAD_PLUS:
		return '+';
	case KEY_SEMICOLON:
		return ';';
	default:
		if (button >= KEY_PAD_0 && button <= KEY_PAD_9) {
			return button - KEY_PAD_0 + '0';
		}
		if (strlen(g_IInputSystem->ButtonCodeToString(button)) != 1) return 0;
		return *g_IInputSystem->ButtonCodeToString(button);
	}
}

void FixMovement(CUserCmd& cmd, Vector& viewangles) {
	Vector movement, ang;
	float speed, yaw;
	movement.x = cmd.forwardmove;
	movement.y = cmd.sidemove;
	movement.z = cmd.upmove;
	speed = sqrt(movement.x * movement.x + movement.y * movement.y);
	VectorAngles(movement, ang);
	yaw = DEG2RAD(ang.y - viewangles.y + cmd.viewangles.y);
	cmd.forwardmove = cos(yaw) * speed;
	cmd.sidemove = sin(yaw) * speed;
}

bool AmbassadorCanHeadshot() {
	if (IsAmbassador(g_pLocalPlayer->weapon())) {
		if ((g_GlobalVars->curtime - CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flLastFireTime)) <= 1.0) {
			return false;
		}
	}
	return true;
}

float RandFloatRange(float min, float max) {
    return (min + 1) + (((float) rand()) / (float) RAND_MAX) * (max - (min + 1));
}

bool IsEntityVisible(CachedEntity* entity, int hb) {
	Vector hit;
	if (g_Settings.bInvalid) return false;
	if (entity == g_pLocalPlayer->entity) return true;
	if (hb == -1) {
		return IsEntityVectorVisible(entity, entity->m_vecOrigin);
	} else {
		return entity->hitboxes.VisibilityCheck(hb);
	}

}

bool IsEntityVectorVisible(CachedEntity* entity, Vector endpos) {
	trace_t trace_object;
	Ray_t ray;

	if (g_Settings.bInvalid) return false;
	if (entity == g_pLocalPlayer->entity) return true;
	if (CE_BAD(g_pLocalPlayer->entity)) return false;
	if (CE_BAD(entity)) return false;
	trace::filter_default.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
	ray.Init(g_pLocalPlayer->v_Eye, endpos);
	{
		PROF_SECTION(IEVV_TraceRay);
		g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_default, &trace_object);
	}
	return (trace_object.fraction >= 0.99f || (((IClientEntity*)trace_object.m_pEnt)) == RAW_ENT(entity));
}

// For when you need to vis check something that isnt the local player
bool VisCheckEntFromEnt(CachedEntity* startEnt, CachedEntity* endEnt) {
	// We setSelf as the starting ent as we dont want to hit it, we want the other ent
    trace_t trace;
    trace::filter_default.SetSelf(RAW_ENT(startEnt));
	
	// Setup the trace starting with the origin of the starting ent attemting to hit the origin of the end ent
    Ray_t ray;
	ray.Init(startEnt->m_vecOrigin, endEnt->m_vecOrigin);
	{
		PROF_SECTION(IEVV_TraceRay);
		g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_default, &trace);
	}
	// Is the entity that we hit our target ent? if so, the vis check passes
    if (trace.m_pEnt) {
        if ((((IClientEntity*)trace.m_pEnt)) == RAW_ENT(endEnt)) return true;
	}
	// Since we didnt hit our target ent, the vis check failed so return false
    return false;
}

// Use when you need to vis check something but its not the ent origin that you use, so we check from the vector to the ent, ignoring the first just in case
bool VisCheckEntFromEntVector(Vector startVector, CachedEntity* startEnt, CachedEntity* endEnt) {
	// We setSelf as the starting ent as we dont want to hit it, we want the other ent
    trace_t trace;
    trace::filter_default.SetSelf(RAW_ENT(startEnt));
	
	// Setup the trace starting with the origin of the starting ent attemting to hit the origin of the end ent
    Ray_t ray;
	ray.Init(startVector, endEnt->m_vecOrigin);
	{
		PROF_SECTION(IEVV_TraceRay);
		g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_default, &trace);
	}
	// Is the entity that we hit our target ent? if so, the vis check passes
    if (trace.m_pEnt) {
        if ((((IClientEntity*)trace.m_pEnt)) == RAW_ENT(endEnt)) return true;
	}
	// Since we didnt hit our target ent, the vis check failed so return false
    return false;
}

Vector GetBuildingPosition(CachedEntity* ent) {
	Vector res;
	res = ent->m_vecOrigin;
	if (ent->m_iClassID == CL_CLASS(CObjectDispenser)) res.z += 30;
	if (ent->m_iClassID == CL_CLASS(CObjectTeleporter)) res.z += 8;
	if (ent->m_iClassID == CL_CLASS(CObjectSentrygun)) {
		switch (CE_INT(ent, netvar.iUpgradeLevel)) {
		case 1:
			res.z += 30;
			break;
		case 2:
			res.z += 50;
			break;
		case 3:
			res.z += 60;
			break;
		}
	}
	return res;
}

bool IsBuildingVisible(CachedEntity* ent) {
	return IsEntityVectorVisible(ent, GetBuildingPosition(ent));
}

void fClampAngle(Vector& qaAng) {
	while(qaAng[0] > 89)
		qaAng[0] -= 180;

	while(qaAng[0] < -89)
		qaAng[0] += 180;

	while(qaAng[1] > 180)
		qaAng[1] -= 360;

	while(qaAng[1] < -180)
		qaAng[1] += 360;

	qaAng.z = 0;
}

float DistToSqr(CachedEntity* entity) {
	if (CE_BAD(entity)) return 0.0f;
	return g_pLocalPlayer->v_Origin.DistToSqr(entity->m_vecOrigin);
}

void Patch(void* address, void* patch, size_t length) {
	void* page = (void*)((uintptr_t)address &~ 0xFFF);
	mprotect(page, 0xFFF, PROT_WRITE | PROT_EXEC);
	memcpy(address, patch, length);
	mprotect(page, 0xFFF, PROT_EXEC);
}

bool IsProjectileCrit(CachedEntity* ent) {
	if (ent->m_bGrenadeProjectile)
		return CE_BYTE(ent, netvar.Grenade_bCritical);
	return CE_BYTE(ent, netvar.Rocket_bCritical);
}

weaponmode GetWeaponMode() {
	int weapon_handle, slot;
	CachedEntity *weapon;

	if (CE_BAD(LOCAL_E)) return weapon_invalid;
	weapon_handle = CE_INT(LOCAL_E, netvar.hActiveWeapon);
	if (IDX_BAD((weapon_handle & 0xFFF))) {
		//logging::Info("IDX_BAD: %i", weapon_handle & 0xFFF);
		return weaponmode::weapon_invalid;
	}
	weapon = (ENTITY(weapon_handle & 0xFFF));
	if (CE_BAD(weapon)) return weaponmode::weapon_invalid;
	slot = vfunc<int(*)(IClientEntity*)>(RAW_ENT(g_pLocalPlayer->weapon()), 395, 0)(RAW_ENT(g_pLocalPlayer->weapon()));
	if (slot == 2) return weaponmode::weapon_melee;
	if (slot > 2) {
		return weaponmode::weapon_pda;
	} else if (weapon->m_iClassID == CL_CLASS(CTFLunchBox) ||
			weapon->m_iClassID == CL_CLASS(CTFLunchBox_Drink) ||
			weapon->m_iClassID == CL_CLASS(CTFBuffItem)) {
		return weaponmode::weapon_consumable;
	} else if ( weapon->m_iClassID == CL_CLASS(CTFRocketLauncher_DirectHit) ||
				weapon->m_iClassID == CL_CLASS(CTFRocketLauncher) ||
				weapon->m_iClassID == CL_CLASS(CTFGrenadeLauncher) ||
				weapon->m_iClassID == CL_CLASS(CTFPipebombLauncher) ||
				weapon->m_iClassID == CL_CLASS(CTFCompoundBow) ||
				weapon->m_iClassID == CL_CLASS(CTFBat_Wood) ||
				weapon->m_iClassID == CL_CLASS(CTFBat_Giftwrap) ||
				weapon->m_iClassID == CL_CLASS(CTFFlareGun) ||
				weapon->m_iClassID == CL_CLASS(CTFFlareGun_Revenge) ||
				weapon->m_iClassID == CL_CLASS(CTFSyringeGun) ||
			    weapon->m_iClassID == CL_CLASS(CTFCrossbow) ||
			    weapon->m_iClassID == CL_CLASS(CTFShotgunBuildingRescue) ||
			    weapon->m_iClassID == CL_CLASS(CTFDRGPomson)) {
		return weaponmode::weapon_projectile;
	} else if (weapon->m_iClassID == CL_CLASS(CTFJar) ||
			   weapon->m_iClassID == CL_CLASS(CTFJarMilk)) {
		return weaponmode::weapon_throwable;
	} else if (weapon->m_iClassID == CL_CLASS(CWeaponMedigun)) {
		return weaponmode::weapon_medigun;
	}
	return weaponmode::weapon_hitscan;
}

bool LineIntersectsBox(Vector& bmin, Vector& bmax, Vector& lmin, Vector& lmax) {
	if (lmax.x < bmin.x && lmin.x < bmin.x) return false;
	if (lmax.y < bmin.y && lmin.y < bmin.y) return false;
	if (lmax.z < bmin.z && lmin.z < bmin.z) return false;
	if (lmax.x > bmax.x && lmin.x > bmax.x) return false;
	if (lmax.y > bmax.y && lmin.y > bmax.y) return false;
	if (lmax.z > bmax.z && lmin.z > bmax.z) return false;
	return true;
}

// TODO add bison and grapple hook
// TODO FIX this function
bool GetProjectileData(CachedEntity* weapon, float& speed, float& gravity) {
	float rspeed, rgrav;

	IF_GAME (!IsTF()) return false;

	if (CE_BAD(weapon)) return false;
	rspeed = 0.0f;
	rgrav = 0.0f;
	typedef float(GetProjectileData)(IClientEntity*);

	if (weapon->m_iClassID == CL_CLASS(CTFRocketLauncher_DirectHit)) {
		rspeed = 1980.0f;
	} else if (weapon->m_iClassID == CL_CLASS(CTFRocketLauncher)) {
		rspeed = 1100.0f;
	} else if (weapon->m_iClassID == CL_CLASS(CTFGrenadeLauncher)) {
		IF_GAME (IsTF2()) {
			rspeed = vfunc<GetProjectileData*>(RAW_ENT(weapon), 527)(RAW_ENT(weapon));
			// TODO Wrong grenade launcher gravity
			rgrav = 0.5f;
		} else IF_GAME (IsTF2C()) {
			rspeed = 1100.0f;
			rgrav = 0.5f;
		}
	} else if (weapon->m_iClassID == CL_CLASS(CTFCompoundBow)) {
		rspeed = vfunc<GetProjectileData*>(RAW_ENT(weapon), 527)(RAW_ENT(weapon));
		rgrav = vfunc<GetProjectileData*>(RAW_ENT(weapon), 528)(RAW_ENT(weapon));
	} else if (weapon->m_iClassID == CL_CLASS(CTFBat_Wood)) {
		rspeed = 3000.0f;
		rgrav = 0.5f;
	} else if (weapon->m_iClassID == CL_CLASS(CTFFlareGun)) {
		rspeed = 2000.0f;
		rgrav = 0.25f;
	} else if (weapon->m_iClassID == CL_CLASS(CTFSyringeGun)) {
		rgrav = 0.2f;
		rspeed = 990.0f;
	} else if (weapon->m_iClassID == CL_CLASS(CTFCrossbow)) {
		rgrav = 0.2f;
		rspeed = 2400.0f;
	} else if (weapon->m_iClassID == CL_CLASS(CTFShotgunBuildingRescue)) {
		rgrav = 0.2f;
		rspeed = 2400.0f;
	} else if (weapon->m_iClassID == CL_CLASS(CTFDRGPomson)) {
		rspeed = 1200.0f;
	}
	speed = rspeed;
	gravity = rgrav;
	return (rspeed || rgrav);
}

constexpr unsigned developer_list[] = { 306902159 };

bool Developer(CachedEntity* ent) {
	/*
	for (int i = 0; i < sizeof(developer_list) / sizeof(unsigned); i++) {
		if (developer_list[i] == ent->player_info.friendsID) return true;
	}
	*/
	return false;
}

/*const char* MakeInfoString(IClientEntity* player) {
	char* buf = new char[256]();
	player_info_t info;
	if (!engineClient->GetPlayerInfo(player->entindex(), &info)) return (const char*)0;
	logging::Info("a");
	int hWeapon = NET_INT(player, netvar.hActiveWeapon);
	if (NET_BYTE(player, netvar.iLifeState)) {
		sprintf(buf, "%s is dead %s", info.name, tfclasses[NET_INT(player, netvar.iClass)]);
		return buf;
	}
	if (hWeapon) {
		IClientEntity* weapon = ENTITY(hWeapon & 0xFFF);
		sprintf(buf, "%s is %s with %i health using %s", info.name, tfclasses[NET_INT(player, netvar.iClass)], NET_INT(player, netvar.iHealth), weapon->GetClientClass()->GetName());
	} else {
		sprintf(buf, "%s is %s with %i health", info.name, tfclasses[NET_INT(player, netvar.iClass)], NET_INT(player, netvar.iHealth));
	}
	logging::Info("Result: %s", buf);
	return buf;
}*/

bool IsVectorVisible(Vector origin, Vector target) {
	trace_t trace_visible;
	Ray_t ray;

	trace::filter_no_player.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
	ray.Init(origin, target);
	g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_no_player, &trace_visible);
	return (trace_visible.fraction == 1.0f);
}

void WhatIAmLookingAt(int* result_eindex, Vector* result_pos) {
	Ray_t ray;
	Vector forward;
	float sp, sy, cp, cy;
	QAngle angle;
	trace_t trace;

	trace::filter_default.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
	g_IEngine->GetViewAngles(angle);
	sy = sinf(DEG2RAD(angle[1]));
	cy = cosf(DEG2RAD(angle[1]));
	sp = sinf(DEG2RAD(angle[0]));
	cp = cosf(DEG2RAD(angle[0]));
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
	forward = forward * 8192.0f + g_pLocalPlayer->v_Eye;
	ray.Init(g_pLocalPlayer->v_Eye, forward);
	g_ITrace->TraceRay(ray, 0x4200400B, &trace::filter_default, &trace);
	if (result_pos)
		*result_pos = trace.endpos;
	if (result_eindex) {
		*result_eindex = 0;
	}
	if (trace.m_pEnt && result_eindex) {
		*result_eindex = ((IClientEntity*)(trace.m_pEnt))->entindex();
	}
}

bool IsSentryBuster(CachedEntity* entity) {
	return (entity->m_Type == EntityType::ENTITY_PLAYER &&
			CE_INT(entity, netvar.iClass) == tf_class::tf_demoman &&
			g_pPlayerResource->GetMaxHealth(entity) == 2500);
}

bool IsAmbassador(CachedEntity* entity) {
	IF_GAME (!IsTF2()) return false;
	if (entity->m_iClassID != CL_CLASS(CTFRevolver)) return false;
	const int& defidx = CE_INT(entity, netvar.iItemDefinitionIndex);
	return (defidx == 61 || defidx == 1006);
}

bool IsPlayerInvulnerable(CachedEntity* player) {
	return HasConditionMask<KInvulnerabilityMask.cond_0, KInvulnerabilityMask.cond_1, KInvulnerabilityMask.cond_2, KInvulnerabilityMask.cond_3>(player);
}

bool IsPlayerCritBoosted(CachedEntity* player) {
	return HasConditionMask<KCritBoostMask.cond_0, KCritBoostMask.cond_1, KCritBoostMask.cond_2, KCritBoostMask.cond_3>(player);
}

bool IsPlayerInvisible(CachedEntity* player) {
	return HasConditionMask<KInvisibilityMask.cond_0, KInvisibilityMask.cond_1, KInvisibilityMask.cond_2, KInvisibilityMask.cond_3>(player);
}

// F1 c&p
Vector CalcAngle(Vector src, Vector dst) {
	Vector AimAngles, delta;
	float hyp;
	delta = src - dst;
	hyp = sqrtf((delta.x * delta.x) + (delta.y * delta.y)); //SUPER SECRET IMPROVEMENT CODE NAME DONUT STEEL
	AimAngles.x = atanf(delta.z / hyp) * RADPI;
	AimAngles.y = atanf(delta.y / delta.x) * RADPI;
	AimAngles.z = 0.0f;
	if(delta.x >= 0.0)
		AimAngles.y += 180.0f;
	return AimAngles;
}

void MakeVector(Vector angle, Vector& vector) {
	float pitch, yaw, tmp;
	pitch = float(angle[0] * PI / 180);
	yaw = float(angle[1] * PI / 180);
	tmp = float(cos(pitch));
	vector[0] = float(-tmp * -cos(yaw));
	vector[1] = float(sin(yaw)*tmp);
	vector[2] = float(-sin(pitch));
}

float GetFov(Vector angle, Vector src, Vector dst) {
	Vector ang, aim;
	float mag, u_dot_v;
	ang = CalcAngle(src, dst);

	MakeVector(angle, aim);
	MakeVector(ang, ang);

	mag = sqrtf(pow(aim.x, 2) + pow(aim.y, 2) + pow(aim.z, 2));
	u_dot_v = aim.Dot(ang);

	return RAD2DEG(acos(u_dot_v / (pow(mag, 2))));
}

bool CanHeadshot() {
	return (g_pLocalPlayer->flZoomBegin > 0.0f && (g_GlobalVars->curtime - g_pLocalPlayer->flZoomBegin > 0.2f));
}

bool CanShoot() {
	float servertime, nextattack;

	servertime = (float)(CE_INT(g_pLocalPlayer->entity, netvar.nTickBase)) * g_GlobalVars->interval_per_tick;
	nextattack = CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flNextPrimaryAttack);
	return nextattack <= servertime;
}

QAngle VectorToQAngle(Vector in) {
	return *(QAngle*)&in;
}

Vector QAngleToVector(QAngle in) {
	return *(Vector*)&in;
}

void AimAt(Vector origin, Vector target, CUserCmd* cmd) {
	Vector angles, tr;
	tr = (target - origin);
	VectorAngles(tr, angles);
	fClampAngle(angles);
	cmd->viewangles = angles;
}

void AimAtHitbox(CachedEntity* ent, int hitbox, CUserCmd* cmd) {
	Vector r;
	r = ent->m_vecOrigin;
	GetHitbox(ent, hitbox, r);
	AimAt(g_pLocalPlayer->v_Eye, r, cmd);
}

bool IsEntityVisiblePenetration(CachedEntity* entity, int hb) {
	trace_t trace_visible;
	Ray_t ray;
	Vector hit;
	int ret;
	bool correct_entity;
	IClientEntity *ent;
	trace::filter_penetration.SetSelf(RAW_ENT(g_pLocalPlayer->entity));
	trace::filter_penetration.Reset();
	ret = GetHitbox(entity, hb, hit);
	if (ret) {
		return false;
	}
	ray.Init(g_pLocalPlayer->v_Origin + g_pLocalPlayer->v_ViewOffset, hit);
	g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_penetration, &trace_visible);
	correct_entity = false;
	if (trace_visible.m_pEnt) {
		correct_entity = ((IClientEntity*)trace_visible.m_pEnt) == RAW_ENT(entity);
	}
	if (!correct_entity) return false;
	g_ITrace->TraceRay(ray, 0x4200400B, &trace::filter_default, &trace_visible);
	if (trace_visible.m_pEnt) {
		ent = (IClientEntity*)trace_visible.m_pEnt;
		if (ent) {
			if (ent->GetClientClass()->m_ClassID == RCC_PLAYER) {
				if (ent == RAW_ENT(entity)) return false;
				if (trace_visible.hitbox >= 0) {
					return true;
				}
			}
		}
	}
	return false;
}

// Used for getting class names
CatCommand print_classnames("debug_print_classnames", "Lists classnames currently available in console", []() {
	
	// Create a tmp ent for the loop
	CachedEntity* ent;
	
	// Go through all the entities
	for (int i = 0; i < HIGHEST_ENTITY; i++) {
		
		// Get an entity
		ent = ENTITY(i);
		// Check for null/dormant
		if (CE_BAD(ent)) continue;
		
		// Print in console, the class name of the ent
		logging::Info(format(RAW_ENT(ent)->GetClientClass()->m_pNetworkName).c_str());
	}

});

void PrintChat(const char* fmt, ...) {
#if not ENABLE_VISUALS
	return;
#endif
	CHudBaseChat* chat = (CHudBaseChat*)g_CHUD->FindElement("CHudChat");
	if (chat) {
		char* buf = new char[1024];
		va_list list;
		va_start(list, fmt);
		vsprintf(buf, fmt, list);
		va_end(list);
		std::unique_ptr<char> str(strfmt("\x07%06X[\x07%06XCAT\x07%06X]\x01 %s", 0x5e3252, 0xba3d9a, 0x5e3252, buf));
		// FIXME DEBUG LOG
		logging::Info("%s", str.get());
		chat->Printf(str.get());
	} else {
	}
}


// You are responsible for delete[]'ing the resulting string.
char* strfmt(const char* fmt, ...) {
	char* buf = new char[1024];
	va_list list;
	va_start(list, fmt);
	vsprintf(buf, fmt, list);
	va_end(list);
	return buf;
}

const char* powerups[] = {
	"STRENGTH",
	"RESISTANCE",
	"VAMPIRE",
	"REFLECT",
	"HASTE",
	"REGENERATION",
	"PRECISION",
	"AGILITY",
	"KNOCKOUT",
	"KING",
	"PLAGUE",
	"SUPERNOVA",
	"CRITS"
};

const std::string classes[] = {
	"Scout",
	"Sniper",
	"Soldier",
	"Demoman",
	"Medic",
	"Heavy",
	"Pyro",
	"Spy",
	"Engineer"
};
