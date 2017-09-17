/*
 * localplayer.cpp
 *
 *  Created on: Oct 15, 2016
 *      Author: nullifiedcat
 */

#include "common.h"
#include "sdk.h"

void LocalPlayer::Update() {
	CachedEntity *wep;

	entity_idx = g_IEngine->GetLocalPlayer();
	entity = ENTITY(entity_idx);
	if (CE_BAD(entity)) {
		team = 0;
		return;
	}
	holding_sniper_rifle = false;
	wep = weapon();
	if (CE_GOOD(wep)) {
		weapon_mode = GetWeaponMode();
		if (wep->m_iClassID == CL_CLASS(CTFSniperRifle) || wep->m_iClassID == CL_CLASS(CTFSniperRifleDecap)) holding_sniper_rifle = true;
	}
	team = CE_INT(entity, netvar.iTeamNum);
	life_state = CE_BYTE(entity, netvar.iLifeState);
	v_ViewOffset = CE_VECTOR(entity, netvar.vViewOffset);
	v_Origin = entity->m_vecOrigin;
	v_Eye = v_Origin + v_ViewOffset;
	clazz = CE_INT(entity, netvar.iClass);
	health = CE_INT(entity, netvar.iHealth);
	this->bUseSilentAngles = false;
	bZoomed = CE_INT(entity, netvar.iFOV) == 20; //!= NET_INT(entity, netvar.iDefaultFOV);
	if (bZoomed) {
		if (flZoomBegin == 0.0f) flZoomBegin = g_GlobalVars->curtime;
	} else {
		flZoomBegin = 0.0f;
	}
}

CachedEntity* LocalPlayer::weapon() {
	int handle, eid;

	if (CE_BAD(entity)) return 0;
	handle = CE_INT(entity, netvar.hActiveWeapon);
	eid = handle & 0xFFF;
	if (IDX_BAD(eid)) return 0;
	return ENTITY(eid);
}

LocalPlayer* g_pLocalPlayer = 0;
