/*
 * helpers.h
 *
 *  Created on: Oct 8, 2016
 *      Author: nullifiedcat
 */

#ifndef HELPERS_H_
#define HELPERS_H_

class CachedEntity;
class IClientEntity;
class ConVar;
class ConCommand;
class CUserCmd;
class CCommand;
struct player_info_s;
class Vector;

class ICvar;
void SetCVarInterface(ICvar* iface);

#define PI 3.14159265358979323846f
#define RADPI 57.295779513082f
//#define DEG2RAD(x) (float)(x) * (float)(PI / 180.0f)

#include "enums.h"
#include "conditions.h"
#include "entitycache.h"
#include "logging.h"

#include "beforecheaders.h"
#include <string>
#include <sstream>
#include <vector>
#include "aftercheaders.h"

#include "sdk.h"

//typedef void ( *FnCommandCallback_t )( const CCommand &command );

// TODO split this shit

std::vector<ConVar*>& RegisteredVarsList();
std::vector<ConCommand*>& RegisteredCommandsList();

void BeginConVars();
void EndConVars();

// Calling source engine functions would get me more accurate result at cost of speed, so idk.
// TODO?

bool IsPlayerInvulnerable(CachedEntity* player);
bool IsPlayerCritBoosted(CachedEntity* player);
bool IsPlayerInvisible(CachedEntity* player);

const char* GetBuildingName(CachedEntity* ent);
Vector GetBuildingPosition(CachedEntity* ent);
bool IsBuildingVisible(CachedEntity* ent);

ConVar* CreateConVar(std::string name, std::string value, std::string help);
ConCommand* CreateConCommand(const char* name, FnCommandCallback_t callback, const char* help);

powerup_type GetPowerupOnPlayer(CachedEntity* player);
// GetHitbox() is being called really frequently.
// It's better if it won't create a new object each time it gets called.
// So it returns a success state, and the values are stored in out reference.
bool GetHitbox(CachedEntity* entity, int hb, Vector& out);
weaponmode GetWeaponMode();

void FixMovement(CUserCmd& cmd, Vector& viewangles);
void VectorAngles(Vector &forward, Vector &angles);

bool IsEntityVisible(CachedEntity* entity, int hb);
bool IsEntityVectorVisible(CachedEntity* entity, Vector endpos);
bool VisCheckEntFromEnt(CachedEntity* startEnt, CachedEntity* endEnt);
bool VisCheckEntFromEntVector(Vector startVector, CachedEntity* startEnt, CachedEntity* endEnt);
	
bool LineIntersectsBox(Vector& bmin, Vector& bmax, Vector& lmin, Vector& lmax);

float DistToSqr(CachedEntity* entity);
void fClampAngle(Vector& qaAng);
//const char* MakeInfoString(IClientEntity* player);
bool GetProjectileData(CachedEntity* weapon, float& speed, float& gravity);
bool IsVectorVisible(Vector a, Vector b);
bool IsSentryBuster(CachedEntity* ent);
char* strfmt(const char* fmt, ...);
// TODO move that to weaponid.h
bool HasWeapon(CachedEntity* ent, int wantedId);
bool IsAmbassador(CachedEntity* ent);
bool HasDarwins(CachedEntity* ent);
bool AmbassadorCanHeadshot();

inline const char* teamname(int team) {
	if (team == 2) return "RED";
	else if (team == 3) return "BLU";
	return "SPEC";
}
extern const std::string classes[10];
inline const char* classname(int clazz) {
	if (clazz > 0 && clazz < 10) {
		return classes[clazz - 1].c_str();
	}
	return "Unknown";
}

void PrintChat(const char* fmt, ...);

void WhatIAmLookingAt(int* result_eindex, Vector* result_pos);

void Patch(void* address, void* patch, size_t length);

void AimAt(Vector origin, Vector target, CUserCmd* cmd);
void AimAtHitbox(CachedEntity* ent, int hitbox, CUserCmd* cmd);
bool IsProjectileCrit(CachedEntity* ent);

QAngle VectorToQAngle(Vector in);
Vector QAngleToVector(QAngle in);

bool CanHeadshot();
bool CanShoot();

char GetUpperChar(ButtonCode_t button);
char GetChar(ButtonCode_t button);

bool IsEntityVisiblePenetration(CachedEntity* entity, int hb);

//void RunEnginePrediction(IClientEntity* ent, CUserCmd *ucmd = NULL);
//void StartPrediction(CUserCmd* cmd);
//void EndPrediction();

float RandFloatRange(float min, float max);

bool Developer(CachedEntity* ent);

Vector CalcAngle(Vector src, Vector dst);
void MakeVector(Vector ang, Vector& out);
float GetFov(Vector ang, Vector src, Vector dst);

void ReplaceString(std::string& input, const std::string& what, const std::string& with_what);

std::pair<float, float> ComputeMove(const Vector& a, const Vector& b);
void WalkTo(const Vector& vector);

std::string GetLevelName();

void format_internal(std::stringstream& stream);
template<typename T, typename... Targs>
void format_internal(std::stringstream& stream, T value, Targs... args) {
	stream << value;
	format_internal(stream, args...);
}
template<typename... Args>
std::string format(const Args&... args) {
	std::stringstream stream;
	format_internal(stream, args...);
	return stream.str();
}

extern const std::string classes[10];
extern const char* powerups[POWERUP_COUNT];

#endif /* HELPERS_H_ */
