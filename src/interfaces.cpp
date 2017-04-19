/*
 * interfaces.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include "common.h"
#include "sharedobj.h"
#include "copypasted/CSignature.h"
#include "sdk.h"

#include <unistd.h>

#include "beforecheaders.h"
#include <string>
#include <sstream>
#include "aftercheaders.h"

#include <steam/isteamclient.h>

//class ISteamFriends002;

IVModelRender* g_IVModelRender = nullptr;
ISteamClient* g_ISteamClient = nullptr;
ISteamFriends* g_ISteamFriends = nullptr;
IVEngineClient013* g_IEngine = nullptr;
vgui::ISurface* g_ISurface = nullptr;
vgui::IPanel* g_IPanel = nullptr;
IClientEntityList* g_IEntityList = nullptr;
ICvar* g_ICvar = nullptr;
IGameEventManager2* g_IEventManager2 = nullptr;
IBaseClientDLL* g_IBaseClient = nullptr;
IEngineTrace* g_ITrace = nullptr;
IVModelInfoClient* g_IModelInfo = nullptr;
IInputSystem* g_IInputSystem = nullptr;
CGlobalVarsBase* g_GlobalVars = nullptr;
IPrediction* g_IPrediction = nullptr;
IGameMovement* g_IGameMovement = nullptr;
IInput* g_IInput = nullptr;
ISteamUser* g_ISteamUser = nullptr;
IAchievementMgr* g_IAchievementMgr = nullptr;
ISteamUserStats* g_ISteamUserStats = nullptr;
IStudioRender* g_IStudioRender = nullptr;
IVDebugOverlay* g_IVDebugOverlay = nullptr;
IMaterialSystemFixed* g_IMaterialSystem = nullptr;
IVRenderView* g_IVRenderView = nullptr;
IMaterialSystem* g_IMaterialSystemHL = nullptr;

template<typename T>
T* BruteforceInterface(std::string name, sharedobj::SharedObject* object, int start) {
	T* result = nullptr;
	std::stringstream stream;
	for (int i = start; i < 100; i++) {
		stream.str("");
		stream << name;
		int zeros = 0;
		if (i < 10) zeros = 2;
		else if (i < 100) zeros = 1;
		for (int j = 0; j < zeros; j++) stream << '0';
		stream << i;
		result = reinterpret_cast<T*>(object->CreateInterface(stream.str()));
		if (result) return result;
	}
	logging::Info("RIP Software: can't create interface %s!", name.c_str());
	exit(0);
	return nullptr;
}

void CreateInterfaces() {
	g_ICvar = BruteforceInterface<ICvar>("VEngineCvar", sharedobj::vstdlib);
	g_IEngine = BruteforceInterface<IVEngineClient013>("VEngineClient", sharedobj::engine);
	g_AppID = g_IEngine->GetAppID();
	g_IEntityList = BruteforceInterface<IClientEntityList>("VClientEntityList", sharedobj::client);
	g_IPanel = BruteforceInterface<vgui::IPanel>("VGUI_Panel", sharedobj::vgui2);
	g_ISteamClient = BruteforceInterface<ISteamClient>("SteamClient", sharedobj::steamclient, 17);
	g_ISurface = BruteforceInterface<vgui::ISurface>("VGUI_Surface", sharedobj::vguimatsurface);
	g_IEventManager2 = BruteforceInterface<IGameEventManager2>("GAMEEVENTSMANAGER", sharedobj::engine, 2);
	g_IBaseClient = BruteforceInterface<IBaseClientDLL>("VClient", sharedobj::client);
	g_ITrace = BruteforceInterface<IEngineTrace>("EngineTraceClient", sharedobj::engine);
	g_IModelInfo = BruteforceInterface<IVModelInfoClient>("VModelInfoClient", sharedobj::engine);
	g_IInputSystem = BruteforceInterface<IInputSystem>("InputSystemVersion", sharedobj::inputsystem);
	g_IStudioRender = BruteforceInterface<IStudioRender>("VStudioRender", sharedobj::studiorender);
	g_IVDebugOverlay = BruteforceInterface<IVDebugOverlay>("VDebugOverlay", sharedobj::engine);
	HSteamPipe sp = g_ISteamClient->CreateSteamPipe();
	HSteamUser su = g_ISteamClient->ConnectToGlobalUser(sp);
	g_IVModelRender = BruteforceInterface<IVModelRender>("VEngineModel", sharedobj::engine, 16);
	g_ISteamFriends = g_ISteamClient->GetISteamFriends(su, sp, "SteamFriends002");
	g_GlobalVars = **(reinterpret_cast<CGlobalVarsBase***>((uintptr_t)11 + gSignatures.GetClientSignature("55 89 E5 83 EC ? 8B 45 08 8B 15 ? ? ? ? F3 0F 10")));
	g_IPrediction = BruteforceInterface<IPrediction>("VClientPrediction", sharedobj::client);
	g_IGameMovement = BruteforceInterface<IGameMovement>("GameMovement", sharedobj::client);
	g_IVRenderView = BruteforceInterface<IVRenderView>("VEngineRenderView", sharedobj::engine);
	g_IMaterialSystem = BruteforceInterface<IMaterialSystemFixed>("VMaterialSystem", sharedobj::materialsystem);
	g_IMaterialSystemHL = (IMaterialSystem*)g_IMaterialSystem;
	if (TF2) {
		g_pScreenSpaceEffects = **(IScreenSpaceEffectManager***)(gSignatures.GetClientSignature("F3 0F 10 83 40 05 00 00 C7 44 24 04 ? ? ? ? 89 34 24 F3 0F 11 44 24 08 E8 ? ? ? ? A1 ? ? ? ? 8B 10 89 04 24 89 74 24 08 C7 44 24 04 ? ? ? ? FF 52 0C A1 ? ? ? ? 8B 10 C7 44 24 04 ? ? ? ? 89 04 24 FF 52 14") + 31);
		g_ppScreenSpaceRegistrationHead = *(CScreenSpaceEffectRegistration***)(gSignatures.GetClientSignature("55 89 E5 53 83 EC 14 8B 1D ? ? ? ? 85 DB 74 25 8D B4 26 00 00 00 00 8B 43 04 85 C0 74 10") + 9);
	} else if (TF2C) {
		logging::Info("FATAL: Signatures not defined for TF2C - Screen Space Effects");
		g_pScreenSpaceEffects = nullptr;
		g_ppScreenSpaceRegistrationHead = nullptr;
	} else if (HL2DM) {
		g_pScreenSpaceEffects = **(IScreenSpaceEffectManager***)(gSignatures.GetClientSignature("FF 52 14 E9 E0 FE FF FF 8D 76 00 A1 ? ? ? ? 8B 5D F4 8B 75 F8 8B 7D FC 8B 10 C7 45 0C ? ? ? ? 89 45 08 8B 42 1C 89 EC 5D FF E0") + 12);
		g_ppScreenSpaceRegistrationHead = *(CScreenSpaceEffectRegistration***)(gSignatures.GetClientSignature("E8 ? ? ? ? 8B 10 C7 44 24 04 ? ? ? ? 89 04 24 FF 52 28 85 C0 75 4B 8B 35 ? ? ? ? 85 F6 74 31 90 8B 5E 04 85 DB 74 22 8B 03 89 1C 24") + 27);
	}
	if (TF2) g_IInput = **(reinterpret_cast<IInput***>((uintptr_t)1 + gSignatures.GetClientSignature("A1 ? ? ? ? C6 05 ? ? ? ? 01 8B 10 89 04 24 FF 92 B4 00 00 00 A1 ? ? ? ? 8B 10")));
	else if (TF2C) g_IInput = **(reinterpret_cast<IInput***>((uintptr_t)1 + gSignatures.GetClientSignature("A1 ? ? ? ? C6 05 ? ? ? ? 01 8B 10 89 04 24 FF 92 A8 00 00 00 A1 ? ? ? ? 8B 10")));
	else if (HL2DM)  g_IInput = **(reinterpret_cast<IInput***>((uintptr_t)1 + gSignatures.GetClientSignature("A1 ? ? ? ? 8B 10 89 04 24 FF 52 78 A1 ? ? ? ? 8B 10")));
	g_ISteamUser = g_ISteamClient->GetISteamUser(su, sp, "SteamUser018");
	g_IAchievementMgr = g_IEngine->GetAchievementMgr();
	g_ISteamUserStats = g_ISteamClient->GetISteamUserStats(su, sp, "STEAMUSERSTATS_INTERFACE_VERSION011");
}
