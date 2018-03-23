
/*
 *
 *	The cathook framework uses its own "entitys" to do its actions.
 *  This allows it to work with any module that can fill the entity manager with entitys
 *
 */

#include "entitys.hpp"

CMFunction<bool(CatEntity*)> GetDormant {[](auto) -> auto { return true; }};
CMFunction<bool(CatEntity*)> GetAlive {[](auto) -> auto { return true; }};
CMFunction<int(CatEntity*)> GetHealth {[](auto) -> auto { return 100; }};
CMFunction<int(CatEntity*)> GetMaxHealth {[](auto) -> auto { return 100; }};
CMFunction<int(CatEntity*)> GetTeam {[](auto) -> int { return ETEAM_NONE; }};
CMFunction<int(CatEntity*)> GetType {[](auto) -> int { return ETYPE_NONE; }};
CMFunction<const char*(CatEntity*)> GetName {[](auto) -> auto { return "unknown"; }};
CMFunction<CatVector(CatEntity*)> GetOrigin {[](auto) -> auto { return CatVector(); }};
CMFunction<CatBox(CatEntity*)> GetCollision {[](auto) -> auto { return CatBox(); }};
CMFunction<int(CatEntity*)> GetSteamId {[](auto) -> auto { return 0; }};
CMFunction<bool(CatEntity*, int, CatBox&)> GetBone {[](CatEntity*, int, CatBox&) -> auto { return false; }};

CMFunction<void(CatLocalPlayer*)> Attack;
CMFunction<void(CatLocalPlayer*)> Jump;
CMFunction<void(CatLocalPlayer*, CatVector)> WalkTo;
CMFunction<bool(CatLocalPlayer*)> InThirdperson {[](auto) -> auto { return false; }};
CMFunction<CatVector(CatLocalPlayer*)> GetCamera {[](auto) -> auto { return CatVector(); }};
CMFunction<CatVector(CatLocalPlayer*)> GetCameraAngle {[](auto) -> auto { return CatVector(); }};
CMFunction<void(CatLocalPlayer*, CatVector)> SetCameraAngle {[](CatLocalPlayer*, CatVector){}};
CMFunction<void(CatLocalPlayer*, CatVector)> SetSilentCameraAngle {[](CatLocalPlayer*, CatVector){}};

CMFunction<int()> GetEntityCount {[]() -> auto { return 0; }};
CMFunction<CatEntity*(int)> GetEntity {[](auto) -> CatEntity* { return nullptr; }};
CMFunction<CatLocalPlayer*()> GetLocalPlayer {[]() -> CatLocalPlayer* { return nullptr; }};
