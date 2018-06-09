/*
a * entity.cpp
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#include "copypasted/Netvar.h"
#include "common.hpp"

NetVars netvar;

void NetVars::Init()
{
    this->iFlags     = gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("m_fFlags"));
    this->iHealth    = gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("m_iHealth"));
    this->iLifeState = gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("m_lifeState"));
    this->iTeamNum   = gNetvars.get_offset(XORSTR("DT_BaseEntity"), XORSTR("m_iTeamNum"));
    this->vViewOffset =
        gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("localdata"), XORSTR("m_vecViewOffset[0]"));
    this->hActiveWeapon =
        gNetvars.get_offset(XORSTR("DT_BaseCombatCharacter"), XORSTR("m_hActiveWeapon"));
    this->hMyWeapons =
        gNetvars.get_offset(XORSTR("DT_BaseCombatCharacter"), XORSTR("m_hMyWeapons"));
    this->iHitboxSet = gNetvars.get_offset(XORSTR("DT_BaseAnimating"), XORSTR("m_nHitboxSet"));
    this->vVelocity =
        gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("localdata"), XORSTR("m_vecVelocity[0]"));
    this->movetype = gNetvars.get_offset(XORSTR("DT_BaseEntity"), XORSTR("movetype"));
    this->m_iAmmo =
        gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("localdata"), XORSTR("m_iAmmo"));
    this->m_iPrimaryAmmoType = gNetvars.get_offset(
        XORSTR("DT_LocalWeaponData"), XORSTR("localdata"), XORSTR("m_iPrimaryAmmoType"));
    this->m_iSecondaryAmmoType = gNetvars.get_offset(
        XORSTR("DT_LocalWeaponData"), XORSTR("localdata"), XORSTR("m_iSecondaryAmmoType"));
    this->m_iClip1 = gNetvars.get_offset(XORSTR("DT_BaseCombatWeapon"),
                                         XORSTR("LocalWeaponData"), XORSTR("m_iClip1"));
    this->m_iClip2 = gNetvars.get_offset(XORSTR("DT_BaseCombatWeapon"),
                                         XORSTR("LocalWeaponData"), XORSTR("m_iClip2"));
    this->m_Collision = gNetvars.get_offset(XORSTR("DT_BaseEntity"), XORSTR("m_Collision"));
    m_flSimulationTime =
        gNetvars.get_offset(XORSTR("DT_BaseEntity"), XORSTR("m_flSimulationTime"));
    IF_GAME(IsTF2())
    {
        res_iTeam =
            gNetvars.get_offset(XORSTR("DT_TFPlayerResource"), XORSTR("baseclass"), XORSTR("m_iTeam"));
        this->res_iMaxBuffedHealth =
            gNetvars.get_offset(XORSTR("DT_TFPlayerResource"), XORSTR("m_iMaxBuffedHealth"));
        m_angEyeAngles = gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("tfnonlocaldata"),
                                             XORSTR("m_angEyeAngles[0]"));
        m_angEyeAnglesLocal = gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("tflocaldata"),
                                                  XORSTR("m_angEyeAngles[0]"));
        this->bGlowEnabled =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_bGlowEnabled"));
        // this->iDecapitations = gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"),
        // XORSTR("m_iDecapitations"));  this->flReloadPriorNextFire =
        // gNetvars.get_offset(XORSTR("DT_TFWeaponBase"), XORSTR("LocalActiveTFWeaponData"),
        // XORSTR("m_flReloadPriorNextFire"));  this->flObservedCritChance =
        // gNetvars.get_offset(XORSTR("DT_TFWeaponBase"), XORSTR("LocalActiveTFWeaponData"),
        // XORSTR("m_flObservedCritChance"));
        this->iItemDefinitionIndex =
            gNetvars.get_offset(XORSTR("DT_EconEntity"), XORSTR("m_AttributeManager"), XORSTR("m_Item"),
                                XORSTR("m_iItemDefinitionIndex"));
        this->AttributeList =
            gNetvars.get_offset(XORSTR("DT_EconEntity"), XORSTR("m_AttributeManager"), XORSTR("m_Item"),
                                XORSTR("m_AttributeList")); // hmmm
        this->flChargeBeginTime = gNetvars.get_offset(
            XORSTR("DT_WeaponPipebombLauncher"), XORSTR("PipebombLauncherLocalData"),
            XORSTR("m_flChargeBeginTime"));
        this->flLastFireTime = gNetvars.get_offset(
            XORSTR("DT_TFWeaponBase"), XORSTR("LocalActiveTFWeaponData"), XORSTR("m_flLastFireTime"));
        this->bDistributed =
            gNetvars.get_offset(XORSTR("DT_CurrencyPack"), XORSTR("m_bDistributed"));
        this->_condition_bits = gNetvars.get_offset(
            XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_ConditionList"), XORSTR("_condition_bits"));
        this->m_flStealthNoAttackExpire =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("tfsharedlocaldata"),
                                XORSTR("m_flStealthNoAttackExpire"));
        this->m_nChargeResistType =
            gNetvars.get_offset(XORSTR("DT_WeaponMedigun"), XORSTR("m_nChargeResistType"));
        this->m_flChargeLevel = gNetvars.get_offset(
            XORSTR("DT_WeaponMedigun"), XORSTR("NonLocalTFWeaponMedigunData"),
            XORSTR("m_flChargeLevel"));
        m_bFeignDeathReady = gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"),
                                                 XORSTR("m_bFeignDeathReady"));
        m_bCarryingObject =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_bCarryingObject"));
        m_nSequence = gNetvars.get_offset(XORSTR("DT_BaseAnimating"), XORSTR("m_nSequence"));
        m_iTauntIndex =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_iTauntIndex"));
        m_iTauntConcept =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_iTauntConcept"));
    }
    this->res_iScore =
        gNetvars.get_offset(XORSTR("DT_TFPlayerResource"), XORSTR("baseclass"), XORSTR("m_iScore"));
    IF_GAME(IsTF())
    {
        this->res_iMaxHealth =
            gNetvars.get_offset(XORSTR("DT_TFPlayerResource"), XORSTR("m_iMaxHealth"));
        this->res_iPlayerClass =
            gNetvars.get_offset(XORSTR("DT_TFPlayerResource"), XORSTR("m_iPlayerClass"));
        this->m_bReadyToBackstab =
            gNetvars.get_offset(XORSTR("DT_TFWeaponKnife"), XORSTR("m_bReadyToBackstab"));
        this->m_bDucked = gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("localdata"),
                                              XORSTR("m_Local"), XORSTR("m_bDucked"));
        this->m_flDuckTimer =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_flDuckTimer"));
        this->iCond =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_nPlayerCond"));
        this->iCond1 =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_nPlayerCondEx"));
        this->iCond2 =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_nPlayerCondEx2"));
        this->iCond3 =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_nPlayerCondEx3"));
        this->iClass =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_PlayerClass"), XORSTR("m_iClass"));
        this->flChargedDamage = gNetvars.get_offset(
            XORSTR("DT_TFSniperRifle"), XORSTR("SniperRifleLocalData"), XORSTR("m_flChargedDamage"));
        this->iUpgradeLevel =
            gNetvars.get_offset(XORSTR("DT_BaseObject"), XORSTR("m_iUpgradeLevel"));
        this->iPipeType =
            gNetvars.get_offset(XORSTR("DT_TFProjectile_Pipebomb"), XORSTR("m_iType"));
        this->iBuildingHealth =
            gNetvars.get_offset(XORSTR("DT_BaseObject"), XORSTR("m_iHealth"));
        this->iBuildingMaxHealth =
            gNetvars.get_offset(XORSTR("DT_BaseObject"), XORSTR("m_iMaxHealth"));
        this->iReloadMode =
            gNetvars.get_offset(XORSTR("DT_TFWeaponBase"), XORSTR("m_iReloadMode"));
        this->Rocket_iDeflected =
            gNetvars.get_offset(XORSTR("DT_TFBaseRocket"), XORSTR("m_iDeflected"));
        this->Grenade_iDeflected =
            gNetvars.get_offset(XORSTR("DT_TFWeaponBaseGrenadeProj"), XORSTR("m_iDeflected"));
        this->nForceTauntCam =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_nForceTauntCam"));
        this->Rocket_bCritical =
            gNetvars.get_offset(XORSTR("DT_TFProjectile_Rocket"), XORSTR("m_bCritical"));
        this->Grenade_bCritical =
            gNetvars.get_offset(XORSTR("DT_TFWeaponBaseGrenadeProj"), XORSTR("m_bCritical"));
        this->angEyeAngles = gNetvars.get_offset(
            XORSTR("DT_TFPlayer"), XORSTR("tfnonlocaldata"), XORSTR("m_angEyeAngles[0]"));
        this->iWeaponState =
            gNetvars.get_offset(XORSTR("DT_WeaponMinigun"), XORSTR("m_iWeaponState"));
        this->flChargeLevel = gNetvars.get_offset(XORSTR("DT_WeaponMedigun"),
                                                  XORSTR("NonLocalTFWeaponMedigunData"),
                                                  XORSTR("m_flChargeLevel"));
        this->bChargeRelease =
            gNetvars.get_offset(XORSTR("DT_WeaponMedigun"), XORSTR("m_bChargeRelease"));
        this->m_nStreaks_Player =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_nStreaks"));
        this->m_nStreaks_Resource =
            gNetvars.get_offset(XORSTR("DT_TFPlayerResource"), XORSTR("m_iStreaks"));
    }
    IF_GAME(IsTF2C())
    {
        this->iCritMult =
            gNetvars.get_offset(XORSTR("DT_TFPlayer"), XORSTR("m_Shared"), XORSTR("m_iCritMult"));
        this->bRespawning =
            gNetvars.get_offset(XORSTR("DT_WeaponSpawner"), XORSTR("m_bRespawning"));
    }
    this->flNextAttack = gNetvars.get_offset(XORSTR("DT_BaseCombatCharacter"),
                                             XORSTR("bcc_localdata"), XORSTR("m_flNextAttack"));
    this->flNextPrimaryAttack =
        gNetvars.get_offset(XORSTR("DT_BaseCombatWeapon"), XORSTR("LocalActiveWeaponData"),
                            XORSTR("m_flNextPrimaryAttack"));
    this->flNextSecondaryAttack =
        gNetvars.get_offset(XORSTR("DT_BaseCombatWeapon"), XORSTR("LocalActiveWeaponData"),
                            XORSTR("m_flNextSecondaryAttack"));
    this->iNextThinkTick = gNetvars.get_offset(
        XORSTR("DT_BaseCombatWeapon"), XORSTR("LocalActiveWeaponData"), XORSTR("m_nNextThinkTick"));
    this->nTickBase =
        gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("localdata"), XORSTR("m_nTickBase"));
    this->vecPunchAngle = gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("localdata"),
                                              XORSTR("m_Local"), XORSTR("m_vecPunchAngle"));
    this->vecPunchAngleVel = gNetvars.get_offset(
        XORSTR("DT_BasePlayer"), XORSTR("localdata"), XORSTR("m_Local"), XORSTR("m_vecPunchAngleVel"));
    this->hThrower = gNetvars.get_offset(XORSTR("DT_BaseGrenade"), XORSTR("m_hThrower"));
    this->iObserverMode =
        gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("m_iObserverMode"));
    this->hObserverTarget =
        gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("m_hObserverTarget"));
    this->deadflag    = gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("pl"), XORSTR("deadflag"));
    this->iFOV        = gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("m_iFOV"));
    this->iDefaultFOV = gNetvars.get_offset(XORSTR("DT_BasePlayer"), XORSTR("m_iDefaultFOV"));
    this->hOwner      = gNetvars.get_offset(XORSTR("DT_BaseCombatWeapon"), XORSTR("m_hOwner"));
    this->m_rgflCoordinateFrame = 0x324;
}

void InitNetVars()
{
    netvar.Init();
}
