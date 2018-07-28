
/*
  Created by Jenny White on 28.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include "common.hpp"
#include "SeedPrediction.hpp"
#if ENABLE_VISUALS
union SDL_Event;
struct SDL_Window;
#endif

#define DECLARE_HOOKED_METHOD(name, rtype, ...)                                \
    namespace types                                                            \
    {                                                                          \
    using name = rtype (*)(__VA_ARGS__);                                       \
    }                                                                          \
    namespace methods                                                          \
    {                                                                          \
    rtype name(__VA_ARGS__);                                                   \
    }                                                                          \
    namespace original                                                         \
    {                                                                          \
    extern types::name name;                                                   \
    }

#define DEFINE_HOOKED_METHOD(name, rtype, ...)                                 \
    types::name original::name{ nullptr };                                     \
    rtype methods::name(__VA_ARGS__)

#define HOOK_ARGS(name)                                                        \
    hooked_methods::methods::name, offsets::name(),                            \
        &hooked_methods::original::name
namespace hooked_methods
{
// FireBullets
DECLARE_HOOKED_METHOD(PreDataUpdate, void, void *, int);
// ClientMode
DECLARE_HOOKED_METHOD(CreateMove, bool, void *, float, CUserCmd *);
DECLARE_HOOKED_METHOD(LevelInit, void, void *, const char *);
DECLARE_HOOKED_METHOD(LevelShutdown, void, void *);
// ClientMode + 4
DECLARE_HOOKED_METHOD(FireGameEvent, void, void *, IGameEvent *);
// IBaseClient
DECLARE_HOOKED_METHOD(DispatchUserMessage, bool, void *, int, bf_read &);
DECLARE_HOOKED_METHOD(IN_KeyEvent, int, void *, int, ButtonCode_t,
                      const char *);
// IInput
DECLARE_HOOKED_METHOD(GetUserCmd, CUserCmd *, IInput *, int);
// INetChannel
DECLARE_HOOKED_METHOD(SendNetMsg, bool, INetChannel *, INetMessage &, bool,
                      bool);
DECLARE_HOOKED_METHOD(CanPacket, bool, INetChannel *);
DECLARE_HOOKED_METHOD(Shutdown, void, INetChannel *, const char *);
DECLARE_HOOKED_METHOD(SendDatagram, int, INetChannel *, bf_write *);
// ISteamFriends
DECLARE_HOOKED_METHOD(GetFriendPersonaName, const char *, ISteamFriends *,
                      CSteamID);
// IEngineVGui
DECLARE_HOOKED_METHOD(Paint, void, IEngineVGui *, PaintMode_t);
// IGameEventManager2
DECLARE_HOOKED_METHOD(FireEvent, bool, IGameEventManager2 *, IGameEvent *,
                      bool);
DECLARE_HOOKED_METHOD(FireEventClientSide, bool, IGameEventManager2 *,
                      IGameEvent *);
DECLARE_HOOKED_METHOD(IsPlayingTimeDemo, bool);
#if ENABLE_VISUALS
// ClientMode
DECLARE_HOOKED_METHOD(OverrideView, void, void *, CViewSetup *);
// g_IEngine
// IVModelRender
DECLARE_HOOKED_METHOD(DrawModelExecute, void, IVModelRender *,
                      const DrawModelState_t &, const ModelRenderInfo_t &,
                      matrix3x4_t *);
// IStudioRender
DECLARE_HOOKED_METHOD(BeginFrame, void, IStudioRender *);
// IBaseClient
DECLARE_HOOKED_METHOD(FrameStageNotify, void, void *, ClientFrameStage_t);
// vgui::IPanel
DECLARE_HOOKED_METHOD(PaintTraverse, void, vgui::IPanel *, unsigned int, bool,
                      bool);
// SDL
DECLARE_HOOKED_METHOD(SDL_GL_SwapWindow, void, SDL_Window *);
DECLARE_HOOKED_METHOD(SDL_PollEvent, int, SDL_Event *);
// IUniformRandomStream
DECLARE_HOOKED_METHOD(RandomInt, int, IUniformRandomStream *, int, int);
#endif
}

// TODO
// wontfix.club
#if 0

#if ENABLE_NULL_GRAPHICS
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

#endif
