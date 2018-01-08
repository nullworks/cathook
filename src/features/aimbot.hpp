
/*
 *
 *	The header file for the aimbot
 *
 *
 */

namespace features { namespace aimbot {

// Auto hitbox function
extern CMFunction<CatVector(CatEntity*)> GetAutoHitbox;

// A function to find a place to aim for on the target
CatVector RetriveAimpoint(CatEntity* entity, int mode);

// For modules to add their target selection stuff, return true if target is acceptable
extern std::vector<bool(*)(CatEntity*)> TargetSelectionModule;

void Init();

}}
