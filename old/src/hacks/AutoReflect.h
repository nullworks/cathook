/*
 * AutoReflect.h
 *
 *  Created on: Nov 18, 2016
 *      Author: nullifiedcat
 */

#ifndef HACKS_AUTOREFLECT_H_
#define HACKS_AUTOREFLECT_H_

#include "../common.h"

namespace hacks { namespace tf { namespace autoreflect {

extern CatVar enabled;
extern CatVar idle_only;
extern CatVar stickies;
extern CatVar max_distance;

void CreateMove();
bool ShouldReflect(CachedEntity* ent);
bool IsEntStickyBomb(CachedEntity* ent);

}}}

#endif /* HACKS_AUTOREFLECT_H_ */
