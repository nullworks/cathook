/*
 * crits.h
 *
 *  Created on: Feb 25, 2017
 *      Author: nullifiedcat
 */

#ifndef CRITS_H_
#define CRITS_H_

class CUserCmd;
class IClientEntity;

struct crithack_saved_state {
	float bucket; // 2612
	bool unknown2831;
	int seed; // 2868
	float time; // 2872
	int unknown2616;
	int unknown2620;
	float unknown2856;
	float unknown2860;
	void Save(IClientEntity* entity);
	void Load(IClientEntity* entity);
};

extern bool weapon_can_crit_last;

bool CritKeyDown();
bool AllowAttacking();
bool RandomCrits();
bool WeaponCanCrit();
bool IsAttackACrit(CUserCmd* cmd);
void ResetCritHack();
void LoadSavedState();
void ModifyCommandNumber();

#include "beforecheaders.h"
#include <unordered_map>
#include "aftercheaders.h"
class CatVar;
extern CatVar experimental_crit_hack;

extern int* g_PredictionRandomSeed;
extern std::unordered_map<int, int> command_number_mod;

//bool CalcIsAttackCritical(IClientEntity* weapon);


#endif /* CRITS_H_ */
