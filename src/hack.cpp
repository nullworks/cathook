/*
 * hack.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include "hack.h"

#include "beforecheaders.h"
#include <vector>
#include <map>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/prctl.h>
//#include <cstring>
#include <unistd.h>
#include <link.h>
#include <unordered_map>
#include <cstring>
#include <memory>
#include "segvcatch/segvcatch.h"
#include <csignal>
#include <sys/sysinfo.h>
#include "aftercheaders.h"

#include <steam/isteamuser.h>
// All Hacks
#include "hacks/hacklist.h"

#include "common.h"
#include "sharedobj.h"
#include "hooks.h"
#include "netmessage.h"
#include "profiler.h"
#include "gui/GUI.h"
//#include "gui/controls.h"
#include "cvwrapper.h"

#include "hooks/hookedmethods.h"

#include "sdk.h"
#include "vfunc.h"
#include "copypasted/CSignature.h"
#include "copypasted/Netvar.h"
#include "CDumper.h"
#include <KeyValues.h>

/*
 *  Credits to josh33901 aka F1ssi0N for butifel F1Public and Darkstorm 2015 Linux
 */

bool hack::shutdown = false;

std::mutex hack::command_stack_mutex;
std::stack<std::string>& hack::command_stack() {
	static std::stack<std::string> stack;
	return stack;
}

void hack::ExecuteCommand(const std::string command) {
	std::lock_guard<std::mutex> guard(hack::command_stack_mutex);
	hack::command_stack().push(command);
}

ConCommand* hack::c_Cat = 0;

void hack::CC_Cat(const CCommand& args) {
	g_ICvar->ConsoleColorPrintf(*reinterpret_cast<Color*>(&colors::blu), "cathook");
	g_ICvar->ConsoleColorPrintf(*reinterpret_cast<Color*>(&colors::white), " by ");
	g_ICvar->ConsoleColorPrintf(*reinterpret_cast<Color*>(&colors::blu), "d4rkc4t\n");
	g_ICvar->ConsoleColorPrintf(*reinterpret_cast<Color*>(&colors::white), "build: " CATHOOK_BUILD_NUMBER " \"" CATHOOK_BUILD_NAME "\"\n");
#if _DEVELOPER
	g_ICvar->ConsoleColorPrintf(*reinterpret_cast<Color*>(&colors::red), "[DEVELOPER BUILD]\n");
#else
	g_ICvar->ConsoleColorPrintf(*reinterpret_cast<Color*>(&colors::red), "Build for user " __DRM_NAME " (Early Access)\n");
#endif
#ifdef __DRM_NOTES
	g_ICvar->ConsoleColorPrintf(*reinterpret_cast<Color*>(&colors::red), "Build notes: " __DRM_NOTES "\n");
#endif
}

typedef bool(handlevent_t)(IMatSystemSurface* thisptr, const InputEvent_t& event);
bool test_handleevent(IMatSystemSurface* thisptr, const InputEvent_t& event) {

}

void hack::Initialize() {
	srand(time(0));
	prctl(PR_SET_DUMPABLE,0,42,42,42);
	sharedobj::LoadAllSharedObjects();
	g_pszTFPath = tf_path_from_maps();
	CreateInterfaces();
	CDumper dumper;
	dumper.SaveDump();
	ClientClass* cc = g_IBaseClient->GetAllClasses();
	FILE* cd = fopen("/tmp/cathook-classdump.txt", "w");
	if (cd) {
		while (cc) {
			fprintf(cd, "[%d] %s\n", cc->m_ClassID, cc->GetName());
			cc = cc->m_pNext;
		}
		fclose(cd);
	}
	if (TF2) g_pClassID = new ClassIDTF2();
	else if (TF2C) g_pClassID = new ClassIDTF2C();
	else if (HL2DM) g_pClassID = new ClassIDHL2DM();
	g_pClassID->Init();
	colors::Init();
	if (TF2) {
		uintptr_t mmmf = (gSignatures.GetClientSignature("C7 44 24 04 09 00 00 00 BB ? ? ? ? C7 04 24 00 00 00 00 E8 ? ? ? ? BA ? ? ? ? 85 C0 B8 ? ? ? ? 0F 44 DA") + 37);
		if (mmmf) {
			unsigned char patch1[] = { 0x89, 0xD3, 0x90 };
			unsigned char patch2[] = { 0x89, 0xC2, 0x90 };
			Patch((void*)mmmf, (void*)patch1, 3);
			Patch((void*)(mmmf + 8), (void*)patch2, 3);
		}
		/*uintptr_t canInspectSig = (gSignatures.GetClientSignature("55 0F 57 C0 89 E5 83 EC 48 8B 45 08 F3 0F 11 04 24 F3 0F 11 45 E8 C7 44 24 10 01 00 00 00 C7 44 24 0C 00 00 00 00 89 44 24 08 C7 44 24 ? ? ? ? ? E8 ? ? ? ? F3 0F 10 45 E8 D9 5D E4 F3 0F 10 4D E4 C9 0F 2F C8 0F 95 C0 C3") + 72);
		if (canInspectSig) {
			unsigned char patch[] = { 0xB0, 0x01, 0x90 };
			Patch((void*)canInspectSig, (void*)patch, 3);
		}*/
	}
	BeginConVars();
	hack::c_Cat = CreateConCommand(CON_NAME, &hack::CC_Cat, "Info");
	g_Settings.Init();
	EndConVars();
	draw::Initialize();
	g_pGUI = new CatGUI();
	g_pGUI->Setup();
	gNetvars.init();
	InitNetVars();
	g_pLocalPlayer = new LocalPlayer();
	g_pPlayerResource = new TFPlayerResource();
	hooks::hkPanel = new hooks::VMTHook();
	hooks::hkPanel->Init(g_IPanel, 0);
	//hooks::hkPanel->HookMethod((void*)&hack::Hk_PaintTraverse, hooks::offPaintTraverse);
	hooks::hkPanel->HookMethod((void*)PaintTraverse_hook, hooks::offPaintTraverse);
	hooks::hkPanel->Apply();
	hooks::hkClientMode = new hooks::VMTHook();
	uintptr_t* clientMode = 0;
	while(!(clientMode = **(uintptr_t***)((uintptr_t)((*(void***)g_IBaseClient)[10]) + 1))) {
		sleep(1);
	}
	//hooks::hkMatSurface = new hooks::VMTHook();
	//hooks::hkMatSurface->Init((void*)matsurface, 0);
	//hooks::hkMatSurface->HookMethod((void*)test_handleevent, 1);
	hooks::hkClientMode->Init((void*)clientMode, 0);
	//hooks::hkClientMode->HookMethod((void*)&hack::Hk_CreateMove, hooks::offCreateMove);
	hooks::hkClientMode->HookMethod((void*)CreateMove_hook, hooks::offCreateMove);
	hooks::hkClientMode->HookMethod((void*)OverrideView_hook, hooks::offOverrideView);
	hooks::hkClientMode->HookMethod((void*)LevelInit_hook, hooks::offLevelInit);
	hooks::hkClientMode->HookMethod((void*)LevelShutdown_hook, hooks::offLevelShutdown);
	hooks::hkClientMode->Apply();
	hooks::hkStudioRender = new hooks::VMTHook();
	hooks::hkStudioRender->Init((void*)g_IStudioRender, 0);
	hooks::hkStudioRender->HookMethod((void*)BeginFrame_hook, hooks::offBeginFrame);
	hooks::hkStudioRender->Apply();
	hooks::hkClient = new hooks::VMTHook();
	hooks::hkClient->Init((void*)g_IBaseClient, 0);
	hooks::hkClient->HookMethod((void*)FrameStageNotify_hook, hooks::offFrameStageNotify);
	hooks::hkClient->HookMethod((void*)DispatchUserMessage_hook, hooks::offFrameStageNotify + 1);
	hooks::hkClient->HookMethod((void*)IN_KeyEvent_hook, hooks::offKeyEvent);
	hooks::hkClient->Apply();
	hooks::hkInput = new hooks::VMTHook();
	hooks::hkInput->Init((void*)g_IInput, 0);
	hooks::hkInput->HookMethod((void*)GetUserCmd_hook, hooks::offGetUserCmd);
	hooks::hkInput->Apply();
	hooks::hkIVModelRender = new hooks::VMTHook();
	hooks::hkIVModelRender->Init(g_IVModelRender, 0);
	hooks::hkIVModelRender->HookMethod((void*)DrawModelExecute_hook, hooks::offDrawModelExecute);
	hooks::hkIVModelRender->Apply();
	if (TF2) g_GlowObjectManager = *reinterpret_cast<CGlowObjectManager**>(gSignatures.GetClientSignature("C1 E0 05 03 05") + 5);
	InitStrings();
	hacks::shared::killsay::Init();
	hack::command_stack().push("exec cat_autoexec");
	hack::command_stack().push("cat_killsay_reload");
	hack::command_stack().push("cat_spam_reload");
	logging::Info("Hooked!");
	playerlist::Load();
	if (TF2 || HL2DM) {
		effect_chams::g_pEffectChams = new CScreenSpaceEffectRegistration("_cathook_chams", &effect_chams::g_EffectChams);
		g_pScreenSpaceEffects->EnableScreenSpaceEffect("_cathook_chams");
		effect_chams::g_EffectChams.Init();
		effect_glow::g_pEffectGlow = new CScreenSpaceEffectRegistration("_cathook_glow", &effect_glow::g_EffectGlow);
		g_pScreenSpaceEffects->EnableScreenSpaceEffect("_cathook_glow");
	}
	//for (CScreenSpaceEffectRegistration* reg = *g_ppScreenSpaceRegistrationHead; reg; reg = reg->m_pNext) {
	//	logging::Info("%s", reg->m_pEffectName);
	//}
	logging::Info("SSE enabled..");
}

void hack::Think() {
	usleep(250000);
}

void hack::Shutdown() {
	if (hack::shutdown) return;
	hack::shutdown = true;
	playerlist::Save();
	logging::Info("Killing hooks..");
	if (hooks::hkPanel) hooks::hkPanel->Kill();
	if (hooks::hkClientMode) hooks::hkClientMode->Kill();
	if (hooks::hkClient) hooks::hkClient->Kill();
	if (hooks::hkMatSurface) hooks::hkMatSurface->Kill();
	if (hooks::hkNetChannel) hooks::hkNetChannel->Kill();
	if (hooks::hkStudioRender) hooks::hkStudioRender->Kill();
	if (hooks::hkInput) hooks::hkInput->Kill();
	if (hooks::hkIVModelRender) hooks::hkIVModelRender->Kill();
	//if (hooks::hkCTFPlayer) hooks::hkCTFPlayer->Kill();
	logging::Info("Unregistering convars..");
	ConVar_Unregister();
	logging::Info("Shutting down killsay...");
	hacks::shared::killsay::Shutdown();
	logging::Info("Success..");
}
