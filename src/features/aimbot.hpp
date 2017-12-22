
/*
 *
 *	The header file for the aimbot
 *
 *
 */

namespace features { namespace aimbot {

// Auto hitbox function
extern CMFunction<CatVector(const CatEntity&)> GetAutoHitbox;

// For modules to add their target selection stuff, return true if target is acceptable
extern std::vector<bool(*)(const CatEntity&)> TargetSelectionModule;

void Init();

}}
