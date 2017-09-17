/*
 * PaintTraverse.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 */

#include "PaintTraverse.h"
#include "../common.h"
#include "../hack.h"
#include "hookedmethods.h"
#include "../segvcatch/segvcatch.h"
#include "../copypasted/CSignature.h"
#include "../profiler.h"
#include "../netmessage.h"

CatVar clean_screenshots(CV_SWITCH, "clean_screenshots", "1", "Clean screenshots", "Don't draw visuals while taking a screenshot");
CatVar disable_visuals(CV_SWITCH, "no_visuals", "0", "Disable ALL drawing", "Completely hides cathook");
CatVar no_zoom(CV_SWITCH, "no_zoom", "0", "Disable scope", "Disables black scope overlay");
CatVar pure_bypass(CV_SWITCH, "pure_bypass", "0", "Pure Bypass", "Bypass sv_pure");
void* pure_orig = nullptr;
void** pure_addr = nullptr;

CatEnum software_cursor_enum({"KEEP", "ALWAYS", "NEVER", "MENU ON", "MENU OFF"});
CatVar software_cursor_mode(software_cursor_enum, "software_cursor_mode", "0", "Software cursor", "Try to change this and see what works best for you");

void PaintTraverse_hook(void* _this, unsigned int vp, bool fr, bool ar) {
	static const PaintTraverse_t original = (PaintTraverse_t)hooks::panel.GetMethod(offsets::PaintTraverse());
	static bool textures_loaded = false;
	static unsigned long panel_focus = 0;
	static unsigned long panel_scope = 0;
	static unsigned long panel_top = 0;
	static bool cur, draw_flag = false;
	static bool call_default = true;
	static ConVar* software_cursor = g_ICvar->FindVar("cl_software_cursor");
	static const char *name;
	static std::string name_s, name_stripped, reason_stripped;
#if DEBUG_SEGV == true
	if (!segvcatch::handler_fpe || !segvcatch::handler_segv) {
		if (!segvcatch::handler_fpe) segvcatch::init_segv();
		if (!segvcatch::handler_segv) segvcatch::init_fpe();
	}
#endif
#if ENABLE_VISUALS == 1
	if (!textures_loaded) {
		textures_loaded = true;
		hacks::tf::radar::Init();
	}
#endif
	if (pure_bypass) {
		if (!pure_addr) {
			pure_addr = *reinterpret_cast<void***>(gSignatures.GetEngineSignature("55 89 E5 83 EC 18 A1 ? ? ? ? 89 04 24 E8 0D FF FF FF A1 ? ? ? ? 85 C0 74 08 89 04 24 E8 ? ? ? ? C9 C3") + 7);
		}
		if (*pure_addr)
			pure_orig = *pure_addr;
		*pure_addr = (void*)0;
	} else if (pure_orig) {
		*pure_addr = pure_orig;
		pure_orig = (void*)0;
	}
	call_default = true;
	if (cathook && panel_scope && no_zoom && vp == panel_scope) call_default = false;

	if (software_cursor_mode) {
		cur = software_cursor->GetBool();
		switch ((int)software_cursor_mode) {
		case 1:
			if (!software_cursor->GetBool()) software_cursor->SetValue(1);
			break;
		case 2:
			if (software_cursor->GetBool()) software_cursor->SetValue(0);
			break;
#if ENABLE_GUI
		case 3:
			if (cur != g_pGUI->Visible()) {
				software_cursor->SetValue(g_pGUI->Visible());
			}
			break;
		case 4:
			if (cur == g_pGUI->Visible()) {
				software_cursor->SetValue(!g_pGUI->Visible());
			}
#endif
		}
	}

	if (call_default) SAFE_CALL(original(_this, vp, fr, ar));
	// To avoid threading problems.

	PROF_SECTION(PT_total);


	if (vp == panel_top) draw_flag = true;
	if (!cathook) return;

	if (!panel_top) {
		name = g_IPanel->GetName(vp);
		if (strlen(name) > 4) {
			if (name[0] == 'M' && name[3] == 'S') {
				panel_top = vp;
			}

		}
	}
	if (!panel_focus) {
		name = g_IPanel->GetName(vp);
		if (strlen(name) > 5) {
			if (name[0] == 'F' && name[5] == 'O') {
				panel_focus = vp;
			}
		}
	}
	if (!panel_scope) {
		name = g_IPanel->GetName(vp);
		if (!strcmp(name, "HudScope")) {
			panel_scope = vp;
		}
	}
	if (!g_IEngine->IsInGame()) {
		g_Settings.bInvalid = true;
	}

	if (vp != panel_focus) return;
	g_IPanel->SetTopmostPopup(panel_focus, true);
	if (!draw_flag) return;
	draw_flag = false;

	if (disable_visuals) return;

	if (clean_screenshots && g_IEngine->IsTakingScreenshot()) return;

	PROF_SECTION(PT_active);
#if ENABLE_VISUALS == 1
	draw::UpdateWTS();
	BeginCheatVisuals();
	DrawCheatVisuals();


#if ENABLE_GUI
		g_pGUI->Update();
#endif


	EndCheatVisuals();
#endif
	SEGV_END;
}

