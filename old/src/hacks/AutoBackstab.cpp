/*
 * AutoBackstab.cpp
 *
 *  Created on: Apr 14, 2017
 *      Author: nullifiedcat
 */

#include "../common.h"
#include "../copypasted/CSignature.h"

namespace hacks { namespace tf2 { namespace autobackstab {

static CatVar enabled(CV_SWITCH, "autobackstab", "0", "Auto Backstab", "Does not depend on triggerbot!");

// TODO improve
void CreateMove() {
	if (!enabled) return;
	if (g_pLocalPlayer->weapon()->m_iClassID != CL_CLASS(CTFKnife)) return;
	trace_t trace;
	IClientEntity* weapon = RAW_ENT(LOCAL_W);

	typedef bool(*IsBehindAndFacingTarget_t)(IClientEntity*, IClientEntity*);
	static auto IsBehindAndFacingTarget_addr = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC 5C 8B 45 08 8B 75 0C 89 04 24 E8 ? ? ? ? 85 C0 89 C3 74 0F 8B 00 89 1C 24 FF 90 E0 02 00 00 84 C0");
	static auto IsBehindAndFacingTarget = reinterpret_cast<IsBehindAndFacingTarget_t>(IsBehindAndFacingTarget_addr);

	// 515 = DoSwingTrace
	// FIXME offset
	if (vfunc<bool(*)(IClientEntity*, trace_t*)>(weapon, 515)(weapon, &trace)) {
		if (trace.m_pEnt && reinterpret_cast<IClientEntity*>(trace.m_pEnt)->GetClientClass()->m_ClassID == RCC_PLAYER) {
			if (NET_INT(trace.m_pEnt, netvar.iTeamNum) != g_pLocalPlayer->team) {
				if (IsBehindAndFacingTarget(weapon, reinterpret_cast<IClientEntity*>(trace.m_pEnt)))
					g_pUserCmd->buttons |= IN_ATTACK;
			}
		}
	}
}

}}}
