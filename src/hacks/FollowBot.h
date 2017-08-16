/*
 * FollowBot.h
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */

#ifdef IPC_ENABLED

#ifndef HACKS_FOLLOWBOT_H_
#define HACKS_FOLLOWBOT_H_

class CatCommand;
class CatVar;
class CachedEntity;

#include "../ipc.h"

namespace hacks { namespace shared { namespace followbot {

enum class EFollowType {
	VECTOR,	// When we need to goto a vector
	ENTITY	// when we dont have a specific steamid, but we still want to follow an entity
};
	
//extern CatCommand move_to_crosshair;
//extern CatCommand follow;
//extern CatCommand follow_entity;
extern CatVar bot;

extern unsigned follow_steamid;
extern int following_idx;

bool IsBot(CachedEntity* entity);
void AddMessageHandlers(ipc::peer_t* peer);
void AfterCreateMove();
#if ENABLE_VISUALS == 1
void Draw();
#endif
void CrumbReset();
void CrumbTopAdd(Vector crumbToAdd);
void CrumbBottomAdd();
void DoWalking();
void DrawFollowbot();
std::pair<float, float> ComputeMove(const Vector& a, const Vector& b);
void WalkTo(const Vector& vector);
unsigned MakeMask();
void SelectEntity(int idx);

}}}

#endif /* HACKS_FOLLOWBOT_H_ */

#endif
