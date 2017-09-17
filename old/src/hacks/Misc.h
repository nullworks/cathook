/*
 * Misc.h
 *
 *  Created on: Nov 5, 2016
 *      Author: nullifiedcat
 */

#ifndef HACKS_MISC_H_
#define HACKS_MISC_H_

#include "../common.h"

namespace hacks { namespace shared { namespace misc {

void CreateMove();
#if ENABLE_VISUALS == 1
void DrawText();
#endif

extern IClientEntity* found_crit_weapon;
extern int found_crit_number;
extern int last_number;

extern CatVar crit_hack_next;
extern CatVar debug_info;
extern CatVar flashlight_spam;
extern CatVar crit_info; // TODO separate
extern CatVar crit_hack;
extern CatVar crit_melee;
extern CatVar crit_suppress;
extern CatVar anti_afk;
extern CatVar tauntslide;
extern CatCommand name;
extern CatCommand save_settings;
extern CatCommand say_lines;
extern CatCommand disconnect;
extern CatCommand schema;
extern CatCommand disconnect_vac;
extern CatCommand set_value;

extern float last_bucket;

}}}

/*class Misc : public IHack {
public:
	Misc();

	virtual void ProcessUserCmd(CUserCmd*) override;
	virtual void Draw() override;

	CatVar* v_bDebugInfo;
	ConCommand* c_Name;
	ConVar* v_bInfoSpam;
	ConVar* v_bFastCrouch;
	CatVar* v_bFlashlightSpam;
	CatVar* v_bMinigunJump; // TF2C
	CatVar* v_bDebugCrits; // TF2C
	CatVar* v_bAntiAFK;
	CatVar* v_bHookInspect;
	CatVar* v_iFakeLag;
	CatVar* v_bCritHack;
	CatVar* v_bTauntSlide;
	CatVar* v_bSuppressCrits;
	//ConVar* v_bDumpEventInfo;
	ConCommand* c_SaveSettings;
	ConCommand* c_Unrestricted;
	ConCommand* c_DumpItemAttributes;
	ConCommand* c_SayLine;
	ConCommand* c_Shutdown;
	ConCommand* c_AddFriend;
	ConCommand* c_AddRage;
	ConCommand* c_DumpVars;
	ConCommand* c_DumpPlayers;
	ConCommand* c_Teamname;
	ConCommand* c_Lockee;
	ConCommand* c_Info;
	ConCommand* c_DumpConds;
	ConCommand* c_Reset;
	ConCommand* c_Disconnect;
	ConCommand* c_Schema;
	ConCommand* c_DisconnectVAC;

	CatVar* v_bCleanChat;
};

void Schema_Reload();
void CC_Misc_Disconnect_VAC();

DECLARE_HACK_SINGLETON(Misc);*/

#endif /* HACKS_MISC_H_ */
