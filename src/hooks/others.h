/*
 * others.h
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 */

#ifndef OTHERS_H_
#define OTHERS_H_

class INetMessage;
class CViewSetup;
class bf_read;
class SDL_Window;
class CatVar;

extern CatVar disconnect_reason;

bool CanPacket_hook(void*);
int IN_KeyEvent_hook(void*, int, int, const char*);
bool SendNetMsg_hook(void*, INetMessage&, bool, bool);
void Shutdown_hook(void*, const char*);
void OverrideView_hook(void*, CViewSetup*);
bool DispatchUserMessage_hook(void*, int, bf_read&);
void FrameStageNotify_hook(void*, int);
void LevelInit_hook(void*, const char*);
void LevelShutdown_hook(void*);


#if ENABLE_NULL_GRAPHICS == 1
typedef ITexture*(*FindTexture_t)(void*, const char*, const char*, bool, int);
typedef IMaterial*(*FindMaterialEx_t)(void*, const char*, const char*, int, bool, const char*);
typedef IMaterial*(*FindMaterial_t)(void*, const char*, const char*, bool, const char*);

/* 70 */ void ReloadTextures_null_hook(void* this_);
/* 71 */ void ReloadMaterials_null_hook(void* this_, const char *pSubString);
/* 73 */ IMaterial *FindMaterial_null_hook(void* this_, char const* pMaterialName, const char *pTextureGroupName, bool complain, const char *pComplainPrefix);
/* 81 */ ITexture *FindTexture_null_hook(void* this_, char const* pTextureName, const char *pTextureGroupName, bool complain, int nAdditionalCreationFlags);
/* 121 */ void ReloadFilesInList_null_hook(void* this_, IFileList *pFilesToReload);
/* 123 */ IMaterial *FindMaterialEx_null_hook(void* this_, char const* pMaterialName, const char *pTextureGroupName, int nContext, bool complain, const char *pComplainPrefix);
#endif

//extern unsigned int* swapwindow_ptr;
//extern unsigned int  swapwindow_orig;

#endif /* OTHERS_H_ */
