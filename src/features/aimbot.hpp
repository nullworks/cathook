
/*
 *
 *	The header file for the aimbot
 *
 *
 */

#include "../util/catvars.hpp"
#include "../util/functional.hpp"
#include "../framework/entitys.hpp"	// Contains entity and bone info

namespace features::aimbot {

// Module related

// Auto hitbox function
extern CMFunction<CatVector(CatEntity*)> GetAutoHitbox;

// For modules to add their target selection stuff, return true if target is acceptable
extern CMFunction<bool(CatEntity*)> TargetSelectionModule;

// TODO, find good name and impliment
//extern CMFunction<bool(CatEntity*)> CanAimModule;

// Used to tell when an weapon can shoot
extern CMFunction<bool()> CanShoot;

// Use to share same aimbot menu ;)
extern CatEnum aimbot_menu;

// Other

// A function to find a place to aim for on the target
CatVector RetrieveAimpoint(CatEntity* entity, int mode);

// esp colors
extern CatEntity* highlight_target;

void Init();

// Debug
extern CatVarInt multipoint;
extern CatVarInt multipoint_ratio;

}
