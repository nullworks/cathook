 
/*
 *
 *	This is a file to put your hooked functions in. Please keep it tidy and try to keep cheat features in their own files!
 *		-Onee
 *
 */

#include "../../util/safecall.hpp"
#include "SDK/sdk.h"
#include "hacks/hooks.h"
#include "offsets.hpp"

#include "hooked.hpp"


// User commands
CUserCmd* g_pUserCmd = nullptr;

bool CreateMove_hook(void* thisptr, float inputSample, CUserCmd* cmd) {
	static CreateMove_t original_method = (CreateMove_t)hooks::clientmode.GetMethod(offsets::CreateMove());
	bool ret = original_method(thisptr, inputSample, cmd);
	if (!cmd) return ret; // Cant do anything without cmd
	g_pUserCmd = cmd; // Update cmd

	/* 
	 *	Do stuff here
	 */
	
	return ret;
}



void PaintTraverse_hook(void* _this, unsigned int vp, bool fr, bool ar) {
	static const PaintTraverse_t original = (PaintTraverse_t)hooks::panel.GetMethod(offsets::PaintTraverse());
	SAFE_CALL(original(_this, vp, fr, ar)); // To avoid threading problems.

	/*draw::UpdateWTS();
	BeginCheatVisuals();
	
	DrawCheatVisuals();

	EndCheatVisuals();*/
}

