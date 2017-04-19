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
#include "../gui/GUI.h"
#include "../segvcatch/segvcatch.h"
#include "../profiler.h"

CatVar clean_screenshots(CV_SWITCH, "clean_screenshots", "1", "Clean screenshots", "Don't draw visuals while taking a screenshot");
CatVar disable_visuals(CV_SWITCH, "no_visuals", "0", "Disable ALL drawing", "Completely hides cathook");
CatVar no_zoom(CV_SWITCH, "no_zoom", "0", "Disable scope", "Disables black scope overlay");
CatVar logo(CV_SWITCH, "logo", "1", "Show logo", "Show cathook text in top left corner");


void PaintTraverse_hook(void* p, unsigned int vp, bool fr, bool ar) {
#if DEBUG_SEGV == true
	if (!segvcatch::handler_fpe || !segvcatch::handler_segv) {
		if (!segvcatch::handler_fpe) segvcatch::init_segv();
		if (!segvcatch::handler_segv) segvcatch::init_fpe();
	}
#endif
	SEGV_BEGIN;
	static bool textures_loaded = false;
	if (!textures_loaded) {
		textures_loaded = true;
		hacks::tf::radar::Init();
	}

	static unsigned long panel_focus = 0;
	static unsigned long panel_scope = 0;
	static unsigned long panel_top = 0;
	static bool draw_flag = false;
	bool call_default = true;
	if (cathook && panel_scope && no_zoom && vp == panel_scope) call_default = false;
	if (cathook) {
		bool vis = gui_visible;
		g_ISurface->SetCursorAlwaysVisible(vis);
	}


	if (call_default) SAFE_CALL(((PaintTraverse_t*)hooks::hkPanel->GetMethod(hooks::offPaintTraverse))(p, vp, fr, ar));
	// To avoid threading problems.

	PROF_SECTION(PaintTraverse);
	if (vp == panel_top) draw_flag = true;
	if (!cathook) return;

	if (!panel_top) {
		const char* name = g_IPanel->GetName(vp);
		if (strlen(name) > 4) {
			if (name[0] == 'M' && name[3] == 'S') {
				panel_top = vp;
			}
			if (name[0] == 'F' && name[5] == 'O') {
				panel_focus = vp;
			}
		}
	}
	if (!panel_scope) {
		if (!strcmp(g_IPanel->GetName(vp), "HudScope")) {
			panel_scope = vp;
		}
	}
	if (!g_IEngine->IsInGame()) {
		g_Settings.bInvalid = true;
	}

	ResetStrings();

	if (vp != panel_focus) return;
	if (!draw_flag) return;
	draw_flag = false;

	{
		std::lock_guard<std::mutex> guard(hack::command_stack_mutex);
		while (!hack::command_stack().empty()) {
			logging::Info("executing %s", hack::command_stack().top().c_str());
			g_IEngine->ClientCmd_Unrestricted(hack::command_stack().top().c_str());
			hack::command_stack().pop();
		}
	}

	if (disable_visuals) return;

	if (clean_screenshots && g_IEngine->IsTakingScreenshot()) return;

	if (logo) {
		AddSideString("cathook by d4rkc4t", colors::RainbowCurrent());
#if defined(GIT_COMMIT_HASH) && defined(GIT_COMMIT_DATE)
		AddSideString("commit #" GIT_COMMIT_HASH);
		AddSideString("at " GIT_COMMIT_DATE);
#endif
	}

	if (!hacks::shared::airstuck::IsStuck()) {
		if (CE_GOOD(g_pLocalPlayer->entity) && !g_Settings.bInvalid) {
			// FIXME
			//if (TF2) SAFE_CALL(hacks::tf2::antibackstab::PaintTraverse());
			if (TF) SAFE_CALL(hacks::tf2::antidisguise::Draw());
			SAFE_CALL(hacks::shared::misc::Draw());
			SAFE_CALL(hacks::shared::esp::Draw());
			if (TF) SAFE_CALL(hacks::tf::spyalert::Draw());
			if (TF) SAFE_CALL(hacks::tf::radar::Draw());
			//hacks::shared::followbot::PrintDebug();
		}
	}


#if GUI_ENABLED == true
		g_pGUI->Update();
#endif

	DrawStrings();
	SEGV_END;
}

