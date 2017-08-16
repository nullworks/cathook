/*
 * entity.h
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#ifndef NETVARS_H_
#define NETVARS_H_

#include "logging.h"

class IClientEntity;

#define NET_VAR(entity, offset, type) \
	(*(reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(entity) + (offset))))

#define NET_INT(entity, offset) \
	NET_VAR(entity, offset, int)

#define NET_FLOAT(entity, offset) \
	NET_VAR(entity, offset, float)

#define NET_BYTE(entity, offset) \
	NET_VAR(entity, offset, unsigned char)

#define NET_VECTOR(entity, offset) \
	NET_VAR(entity, offset, Vector)

typedef unsigned int offset_t;

/*template<typename T>
inline T GetVar(IClientEntity* ent, unsigned int offset) {
	int nullv = 0;
	if (ent == 0) return *(reinterpret_cast<T*>(&nullv));
	//logging::Info("GetEntityValue 0x%08x, 0x%08x", ent, offset);
	return *(reinterpret_cast<T*>((unsigned int)ent + offset));
}

template<typename T>
void SetVar(IClientEntity* ent, unsigned int offset, T value) {
	*(reinterpret_cast<T*>((unsigned int)ent + offset)) = value;
}*/

void InitNetVars();

class NetVars {
public:
	void Init();
	offset_t iTeamNum;
	offset_t iFlags;
	offset_t iHealth;
	offset_t iLifeState;
	offset_t iCond;
	offset_t iCond1;
	offset_t iCond2;
	offset_t iCond3;
	offset_t iClass;
	offset_t vViewOffset;
	offset_t hActiveWeapon;
	offset_t flChargedDamage;
	offset_t iUpgradeLevel;
	offset_t iPipeType;
	offset_t iBuildingHealth;
	offset_t iBuildingMaxHealth;
	offset_t m_iAmmo;
	offset_t iHitboxSet;
	offset_t vVelocity;
	offset_t bGlowEnabled;
	offset_t movetype;
	offset_t iGlowIndex;
	offset_t iReloadMode;
	offset_t res_iMaxHealth;
	offset_t flNextAttack;
	offset_t iNextMeleeCrit;
	offset_t flNextPrimaryAttack;
	offset_t iNextThinkTick;
	offset_t m_iClip1;
	//offset_t flReloadPriorNextFire;
	//offset_t flObservedCritChance;
	offset_t nTickBase;
	//offset_t iDecapitations;
	offset_t res_iMaxBuffedHealth;
	offset_t bRespawning;
	offset_t iItemDefinitionIndex;
	offset_t AttributeList;

	offset_t vecPunchAngle;
	offset_t vecPunchAngleVel;

	offset_t iObserverMode;
	offset_t hObserverTarget;

	offset_t flChargeBeginTime;
	offset_t flLastFireTime;
	offset_t hThrower;
	offset_t hMyWeapons;

	offset_t Rocket_iDeflected;
	offset_t Grenade_iDeflected;
	offset_t Rocket_bCritical;
	offset_t Grenade_bCritical;

	offset_t bDistributed;

	offset_t angEyeAngles;
	offset_t deadflag;
	offset_t nForceTauntCam;

	offset_t iDefaultFOV;
	offset_t iFOV;
	offset_t _condition_bits;
	offset_t res_iPlayerClass;

	offset_t hOwner;
	offset_t iWeaponState;
	offset_t iCritMult; // TF2C

	offset_t flChargeLevel;
	offset_t bChargeRelease;

	offset_t m_flStealthNoAttackExpire;
	offset_t m_flDuckTimer;
	offset_t m_bDucked;
	offset_t m_angEyeAngles;
	offset_t m_bReadyToBackstab;
	offset_t m_Collision;
	offset_t res_iTeam;
	offset_t res_iScore;
	offset_t m_nChargeResistType;
	offset_t m_flChargeLevel;

	offset_t m_rgflCoordinateFrame;
	offset_t m_bFeignDeathReady;
	offset_t m_bCarryingObject;

	offset_t m_iTauntConcept;
	offset_t m_iTauntIndex;
	offset_t m_angEyeAnglesLocal;
	offset_t m_nSequence;
	offset_t m_flSimulationTime;
};

extern NetVars netvar;

#endif /* NETVARS_H_ */
