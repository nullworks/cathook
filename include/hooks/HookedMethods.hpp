
/*
  Created by Jenny White on 28.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include "common.hpp"
#if ENABLE_VISUALS
union SDL_Event;
struct SDL_Window;
#endif

#define DECLARE_HOOKED_METHOD(name, rtype, ...) \
    namespace types                             \
    {                                           \
    using name = rtype (*)(__VA_ARGS__);        \
    }                                           \
    namespace methods                           \
    {                                           \
    rtype name(__VA_ARGS__);                    \
    }                                           \
    namespace original                          \
    {                                           \
    extern types::name name;                    \
    }

#define DEFINE_HOOKED_METHOD(name, rtype, ...) \
    types::name original::name{ nullptr };     \
    rtype methods::name(__VA_ARGS__)

#define HOOK_ARGS(name) hooked_methods::methods::name, offsets::name(), &hooked_methods::original::name
namespace hooked_methods
{
// ClientMode
DECLARE_HOOKED_METHOD(CreateMove, bool, void *, float, CUserCmd *);
DECLARE_HOOKED_METHOD(LevelInit, void, void *, const char *);
DECLARE_HOOKED_METHOD(LevelShutdown, void, void *);
// ClientMode + 4
DECLARE_HOOKED_METHOD(FireGameEvent, void, void *, IGameEvent *);
// IBaseClient
DECLARE_HOOKED_METHOD(DispatchUserMessage, bool, void *, int, bf_read &);
DECLARE_HOOKED_METHOD(IN_KeyEvent, int, void *, int, ButtonCode_t, const char *);
DECLARE_HOOKED_METHOD(FrameStageNotify, void, void *, ClientFrameStage_t);
// IInput
DECLARE_HOOKED_METHOD(CreateMoveInput, void, IInput *, int, float, bool)
DECLARE_HOOKED_METHOD(GetUserCmd, CUserCmd *, IInput *, int);
// INetChannel
DECLARE_HOOKED_METHOD(SendNetMsg, bool, INetChannel *, INetMessage &, bool, bool);
DECLARE_HOOKED_METHOD(CanPacket, bool, INetChannel *);
DECLARE_HOOKED_METHOD(Shutdown, void, INetChannel *, const char *);
DECLARE_HOOKED_METHOD(SendDatagram, int, INetChannel *, bf_write *);
// ISteamFriends
DECLARE_HOOKED_METHOD(GetFriendPersonaName, const char *, ISteamFriends *, CSteamID);
// IEngineVGui
DECLARE_HOOKED_METHOD(Paint, void, IEngineVGui *, PaintMode_t);
// IGameEventManager2
DECLARE_HOOKED_METHOD(FireEvent, bool, IGameEventManager2 *, IGameEvent *, bool);
DECLARE_HOOKED_METHOD(FireEventClientSide, bool, IGameEventManager2 *, IGameEvent *);
// g_IEngine
DECLARE_HOOKED_METHOD(IsPlayingTimeDemo, bool, void *);
DECLARE_HOOKED_METHOD(ServerCmdKeyValues, void, IVEngineClient013 *, KeyValues *);
#if ENABLE_VISUALS || ENABLE_TEXTMODE
// vgui::IPanel
DECLARE_HOOKED_METHOD(PaintTraverse, void, vgui::IPanel *, unsigned int, bool, bool);
#endif
// IUniformRandomStream
DECLARE_HOOKED_METHOD(RandomInt, int, IUniformRandomStream *, int, int);
#if ENABLE_VISUALS
// CHudChat
DECLARE_HOOKED_METHOD(StartMessageMode, int, CHudBaseChat *, int);
DECLARE_HOOKED_METHOD(StopMessageMode, void *, CHudBaseChat *_this);
DECLARE_HOOKED_METHOD(ChatPrintf, void, CHudBaseChat *, int, int, const char *, ...);
// ClientMode
DECLARE_HOOKED_METHOD(OverrideView, void, void *, CViewSetup *);
// IStudioRender
DECLARE_HOOKED_METHOD(BeginFrame, void, IStudioRender *);
// SDL
DECLARE_HOOKED_METHOD(SDL_GL_SwapWindow, void, SDL_Window *);
DECLARE_HOOKED_METHOD(SDL_PollEvent, int, SDL_Event *);
#if ENABLE_CLIP
DECLARE_HOOKED_METHOD(SDL_SetClipboardText, int, const char *);
#endif
#endif
#if ENABLE_VISUALS || ENABLE_TEXTMODE
DECLARE_HOOKED_METHOD(DrawModelExecute, void, IVModelRender *, const DrawModelState_t &, const ModelRenderInfo_t &, matrix3x4_t *);
#endif
// CTFPlayerInventory
DECLARE_HOOKED_METHOD(GetMaxItemCount, int, int *);
// g_ISoundEngine
DECLARE_HOOKED_METHOD(EmitSound1, void, void *, IRecipientFilter &, int, int, const char *, float, float, int, int, int, const Vector *, const Vector *, CUtlVector<Vector> *, bool, float, int);
DECLARE_HOOKED_METHOD(EmitSound2, void, void *, IRecipientFilter &, int, int, const char *, float, soundlevel_t, int, int, int, const Vector *, const Vector *, CUtlVector<Vector> *, bool, float, int);
DECLARE_HOOKED_METHOD(EmitSound3, void, void *, IRecipientFilter &, int, int, int, float, soundlevel_t, int, int, int, const Vector *, const Vector *, CUtlVector<Vector> *, bool, float, int);
// g_IPrediction
DECLARE_HOOKED_METHOD(RunCommand, void, IPrediction *, IClientEntity *, CUserCmd *, IMoveHelper *);
} // namespace hooked_methods

// TODO
// wontfix.club
#if 0

typedef ITexture *(*FindTexture_t)(void *, const char *, const char *, bool,
                                   int);
typedef IMaterial *(*FindMaterialEx_t)(void *, const char *, const char *, int,
                                       bool, const char *);
typedef IMaterial *(*FindMaterial_t)(void *, const char *, const char *, bool,
                                     const char *);

/* 70 */ void ReloadTextures_null_hook(void *this_);
/* 71 */ void ReloadMaterials_null_hook(void *this_, const char *pSubString);
/* 73 */ IMaterial *FindMaterial_null_hook(void *this_,
                                           char const *pMaterialName,
                                           const char *pTextureGroupName,
                                           bool complain,
                                           const char *pComplainPrefix);
/* 81 */ ITexture *FindTexture_null_hook(void *this_, char const *pTextureName,
                                         const char *pTextureGroupName,
                                         bool complain,
                                         int nAdditionalCreationFlags);
/* 121 */ void ReloadFilesInList_null_hook(void *this_,
                                           IFileList *pFilesToReload);
/* 123 */ IMaterial *FindMaterialEx_null_hook(void *this_,
                                              char const *pMaterialName,
                                              const char *pTextureGroupName,
                                              int nContext, bool complain,
                                              const char *pComplainPrefix);

#endif
