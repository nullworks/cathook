#pragma once
#include "common.hpp"
struct BoneCache;
typedef BoneCache *(*GetBoneCache_t)(unsigned);
typedef void (*BoneCacheUpdateBones_t)(BoneCache *, matrix3x4_t *bones, unsigned, float time);
extern unsigned int hitbox_bone_cache_handle_offset;
extern GetBoneCache_t studio_get_bone_cache;
extern BoneCacheUpdateBones_t bone_cache_update_bones;