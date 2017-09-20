 
/*
 *
 *	This is a file to put your hooked functions in. Please keep it tidy and try to keep cheat features in their own files!
 *		-Onee
 *
 */

#include "../../util/safecall.hpp"
#include "../../managers/drawmgr.hpp"

#include "hacks/interfaces.h"
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
	// Storage of our focus things
	static unsigned long panel_focus = 0;
	static unsigned long panel_top = 0;
	
	/*if (!panel_top) {
		name = g_IPanel->GetName(vp);
		if (strlen(name) > 4) {
			if (name[0] == 'M' && name[3] == 'S') {
				panel_top = vp;
			}

		}
	}*/
	if (!panel_focus) {
		char* name = g_IPanel->GetName(vp);
		if (strlen(name) > 5) {
			if (name[0] == 'F' && name[5] == 'O') {
				panel_focus = vp;
			}
		}
	}
	
	//if (vp == panel_top) draw_flag = true;
	if (vp != panel_focus) return;
	g_IPanel->SetTopmostPopup(panel_focus, true);
	/*if (!draw_flag) return;
	draw_flag = false;*/
	
	drawmgr::DrawTick(); // Call the managers draw tick
	/*draw::UpdateWTS();*/
}

