
/*
 *
 *	The header file for the aimbot
 *
 *
 */

#include "../framework/entitys.hpp"	// Contains entity and bone info

namespace features { namespace aimbot {

// Auto hitbox function
extern CMFunction<CatVector(CatEntity*)> GetAutoHitbox;

// A function to find a place to aim for on the target
CatVector RetrieveAimpoint(CatEntity* entity, int mode);

// For modules to add their target selection stuff, return true if target is acceptable
extern std::vector<bool(*)(CatEntity*)> TargetSelectionModule;

extern CatEntity* highlight_target;

void Init();

}}
