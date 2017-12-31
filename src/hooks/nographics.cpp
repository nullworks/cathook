/*
 * nographics.cpp
 *
 *  Created on: Aug 1, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

/*// 81
ITexture *FindTexture_null_hook(void* this_, char const* pTextureName, const
char *pTextureGroupName, bool complain, int nAdditionalCreationFlags) { static
ITexture *st = ((FindTexture_t)hooks::materialsystem.GetMethod(81))(this_,
pTextureName, pTextureGroupName, complain, nAdditionalCreationFlags); return st;
}

// 123
IMaterial *FindMaterialEx_null_hook(void* this_, char const* pMaterialName,
const char *pTextureGroupName, int nContext, bool complain, const char
*pComplainPrefix) { static IMaterial *st =
((FindMaterialEx_t)hooks::materialsystem.GetMethod(123))(this_, pMaterialName,
pTextureGroupName, nContext, complain, pComplainPrefix); return st;
}

// 73
IMaterial *FindMaterial_null_hook(void* this_, char const* pMaterialName, const
char *pTextureGroupName, bool complain, const char *pComplainPrefix) { static
IMaterial *st = ((FindMaterial_t)hooks::materialsystem.GetMethod(73))(this_,
pMaterialName, pTextureGroupName, complain, pComplainPrefix); return st;
}

void ReloadTextures_null_hook(void* this_) {}
void ReloadMaterials_null_hook(void* this_, const char *pSubString) {}
void ReloadFilesInList_null_hook(void* this_, IFileList *pFilesToReload) {}
*/
