/*
 * conditions.h
 *
 *  Created on: Jan 15, 2017
 *      Author: nullifiedcat
 */

#ifndef CONDITIONS_H_
#define CONDITIONS_H_

#include "netvars.h"
#include "entitycache.h"
#include "gameinfo.hpp"

// So, tf2 actually stores cond netvars sequentionally, that's pretty good.
// This struct is just used for easy access to 128bit set, you shouldn't make an instance of it.

struct condition_data_s {
	uint32_t cond_0;
	uint32_t cond_1;
	uint32_t cond_2;
	uint32_t cond_3;
};

enum condition : unsigned {
	TFCond_Slowed = 0,
	TFCond_Zoomed,
	TFCond_Disguising,
	TFCond_Disguised,
	TFCond_Cloaked,
	TFCond_Ubercharged,
	TFCond_TeleportedGlow,
	TFCond_Taunting,
	TFCond_UberchargeFading,
	TFCond_Unknown1, //9
	TFCond_CloakFlicker = 9,
	TFCond_Teleporting,
	TFCond_Kritzkrieged,
	TFCond_Unknown2, //12
	TFCond_TmpDamageBonus = 12,
	TFCond_DeadRingered,
	TFCond_Bonked,
	TFCond_Dazed,
	TFCond_Buffed,
	TFCond_Charging,
	TFCond_DemoBuff,
	TFCond_CritCola,
	TFCond_InHealRadius,
	TFCond_Healing,
	TFCond_OnFire,
	TFCond_Overhealed,
	TFCond_Jarated,
	TFCond_Bleeding,
	TFCond_DefenseBuffed,
	TFCond_Milked,
	TFCond_MegaHeal,
	TFCond_RegenBuffed,
	TFCond_MarkedForDeath,
	TFCond_NoHealingDamageBuff,
	TFCond_SpeedBuffAlly, // 32
	TFCond_HalloweenCritCandy,
	TFCond_CritCanteen,
	TFCond_CritDemoCharge,
	TFCond_CritHype,
	TFCond_CritOnFirstBlood,
	TFCond_CritOnWin,
	TFCond_CritOnFlagCapture,
	TFCond_CritOnKill,
	TFCond_RestrictToMelee,
	TFCond_DefenseBuffNoCritBlock,
	TFCond_Reprogrammed,
	TFCond_CritMmmph,
	TFCond_DefenseBuffMmmph,
	TFCond_FocusBuff,
	TFCond_DisguiseRemoved,
	TFCond_MarkedForDeathSilent,
	TFCond_DisguisedAsDispenser,
	TFCond_Sapped,
	TFCond_UberchargedHidden,
	TFCond_UberchargedCanteen,
	TFCond_HalloweenBombHead,
	TFCond_HalloweenThriller,
	TFCond_RadiusHealOnDamage,
	TFCond_CritOnDamage,
	TFCond_UberchargedOnTakeDamage,
	TFCond_UberBulletResist,
	TFCond_UberBlastResist,
	TFCond_UberFireResist,
	TFCond_SmallBulletResist,
	TFCond_SmallBlastResist,
	TFCond_SmallFireResist,
	TFCond_Stealthed, // 64
	TFCond_MedigunDebuff,
	TFCond_StealthedUserBuffFade,
	TFCond_BulletImmune,
	TFCond_BlastImmune,
	TFCond_FireImmune,
	TFCond_PreventDeath,
	TFCond_MVMBotRadiowave,
	TFCond_HalloweenSpeedBoost,
	TFCond_HalloweenQuickHeal,
	TFCond_HalloweenGiant,
	TFCond_HalloweenTiny,
	TFCond_HalloweenInHell,
	TFCond_HalloweenGhostMode,
	TFCond_MiniCritOnKill,
	TFCond_DodgeChance, //79
	TFCond_ObscuredSmoke = 79,
	TFCond_Parachute,
	TFCond_BlastJumping,
	TFCond_HalloweenKart,
	TFCond_HalloweenKartDash,
	TFCond_BalloonHead,
	TFCond_MeleeOnly,
	TFCond_SwimmingCurse,
	TFCond_HalloweenKartNoTurn, //87
	TFCond_FreezeInput = 87,
	TFCond_HalloweenKartCage,
	TFCond_HasRune,
	TFCond_RuneStrength,
	TFCond_RuneHaste,
	TFCond_RuneRegen,
	TFCond_RuneResist,
	TFCond_RuneVampire,
	TFCond_RuneWarlock,
	TFCond_RunePrecision, // 96
	TFCond_RuneAgility,
	TFCond_GrapplingHook,
	TFCond_GrapplingHookSafeFall,
	TFCond_GrapplingHookLatched,
	TFCond_GrapplingHookBleeding,
	TFCond_AfterburnImmune,
	TFCond_RuneKnockout,
	TFCond_RuneImbalance,
	TFCond_CritRuneTemp,
	TFCond_PasstimeInterception,
	TFCond_SwimmingNoEffects,
	TFCond_EyeaductUnderworld,
	TFCond_KingRune,
	TFCond_PlagueRune,
	TFCond_SupernovaRune,
	TFCond_Plague,
	TFCond_KingAura,
	TFCond_SpawnOutline, //114
	TFCond_KnockedIntoAir,
	TFCond_CompetitiveWinner,
	TFCond_CompetitiveLoser,
	TFCond_NoTaunting
};

template<typename... ConditionList>
constexpr condition_data_s CreateConditionMask(ConditionList... conds) {
	uint32_t
			c0 = 0,
			c1 = 0,
			c2 = 0,
			c3 = 0;
	for (const auto& cond : { conds... }) {
		if (cond >= 32 * 3) {
			c3 |= (1u << (cond % 32));
		}
		if (cond >= 32 * 2) {
			c2 |= (1u << (cond % 32));
		}
		if (cond >= 32 * 1) {
			c1 |= (1u << (cond % 32));
		}
		else {
			c0 |= (1u << (cond));
		}
	}
	return condition_data_s { c0, c1, c2, c3 };
}

// Should be either expanded or unused
constexpr condition_data_s KInvisibilityMask = CreateConditionMask(TFCond_Cloaked);
// Original name
constexpr condition_data_s KVisibilityMask = CreateConditionMask(
		TFCond_OnFire, TFCond_Jarated, TFCond_CloakFlicker,
		TFCond_Milked, TFCond_Bleeding);
constexpr condition_data_s KInvulnerabilityMask = CreateConditionMask(
		TFCond_Ubercharged, TFCond_UberchargedCanteen,
		TFCond_UberchargedHidden, TFCond_UberchargedOnTakeDamage,
		TFCond_Bonked, TFCond_DefenseBuffMmmph);
constexpr condition_data_s KCritBoostMask = CreateConditionMask(
		TFCond_Kritzkrieged, TFCond_CritRuneTemp, TFCond_CritCanteen,
		TFCond_CritMmmph, TFCond_CritOnKill, TFCond_CritOnDamage,
		TFCond_CritOnFirstBlood, TFCond_CritOnWin, TFCond_CritRuneTemp,
		TFCond_HalloweenCritCandy);

// Compiler will optimize this to extremely small functions I guess.
// These functions are never used with dynamic "cond" value anyways.

template<condition cond>
inline bool CondBitCheck(condition_data_s& data) {
	if (cond >= 32 * 3) {
		return data.cond_3 & (1u << (cond % 32));
	}
	if (cond >= 32 * 2) {
		return data.cond_2 & (1u << (cond % 32));
	}
	if (cond >= 32 * 1) {
		return data.cond_1 & (1u << (cond % 32));
	}
	if (cond < 32) {
		return data.cond_0 & (1u << (cond));
	}
	return false;
}

// I haven't figured out how to pass a struct as a parameter, sorry.
template<uint32_t c0, uint32_t c1, uint32_t c2, uint32_t c3>
inline bool CondMaskCheck(condition_data_s& data) {
	return    (data.cond_0 & c0)
		   || (data.cond_1 & c1)
		   || (data.cond_2 & c2)
		   || (data.cond_3 & c3);
}


template<uint32_t c0, uint32_t c1, uint32_t c2, uint32_t c3>
inline bool HasConditionMask(CachedEntity* ent) {
	IF_GAME (!IsTF()) return false;
	IF_GAME (IsTF2()) {
		if (CondMaskCheck<c0, c1, c2, c3>(CE_VAR(ent, netvar._condition_bits, condition_data_s))) return true;
	}
	return CondMaskCheck<c0, c1, c2, c3>(CE_VAR(ent, netvar.iCond, condition_data_s));
}

template<condition cond, bool state>
inline void CondBitSet(condition_data_s& data) {
	if (state) {
		if (cond > 32 * 3) {
			data.cond_3 |= (1 << (cond % 32));
		} else	if (cond > 32 * 2) {
			data.cond_2 |= (1 << (cond % 32));
		} else if (cond > 32 * 1) {
			data.cond_1 |= (1 << (cond % 32));
		} else {
			data.cond_0 |= (1 << (cond));
		}
	} else {
		if (cond > 32 * 3) {
			data.cond_3 &= ~(1u << (cond % 32));
		} else	if (cond > 32 * 2) {
			data.cond_2 &= ~(1u << (cond % 32));
		} else if (cond > 32 * 1) {
			data.cond_1 &= ~(1u << (cond % 32));
		} else {
			data.cond_0 &= ~(1u << (cond));
		}
	}
}

template<condition cond>
inline bool HasCondition(CachedEntity* ent) {
	IF_GAME (!IsTF()) return false;
	IF_GAME (IsTF2() && cond < condition(96)) {
		if (CondBitCheck<cond>(CE_VAR(ent, netvar._condition_bits, condition_data_s))) return true;
	}
	return CondBitCheck<cond>(CE_VAR(ent, netvar.iCond, condition_data_s));
}

template<condition cond>
inline void AddCondition(CachedEntity* ent) {
	IF_GAME (!IsTF()) return;
	IF_GAME (IsTF2()) {
		CondBitSet<cond, true>(CE_VAR(ent, netvar._condition_bits, condition_data_s));
	}
	CondBitSet<cond, true>(CE_VAR(ent, netvar.iCond, condition_data_s));
}

template<condition cond>
inline void RemoveCondition(CachedEntity* ent) {
	IF_GAME (!IsTF()) return;
	IF_GAME (IsTF2()) {
		CondBitSet<cond, false>(CE_VAR(ent, netvar._condition_bits, condition_data_s));
	}
	CondBitSet<cond, false>(CE_VAR(ent, netvar.iCond, condition_data_s));
}

#endif /* CONDITIONS_H_ */
