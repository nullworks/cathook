/*
 * AutoSticky.h
 *
 *  Created on: Dec 2, 2016
 *      Author: nullifiedcat
 */

#ifndef HACKS_AUTOSTICKY_H_
#define HACKS_AUTOSTICKY_H_

#include "../common.h"

namespace hacks { namespace tf { namespace autosticky {

extern CatVar enabled;
extern CatVar buildings;
extern CatVar distance;

bool ShouldDetonate(CachedEntity* bomb);
void CreateMove();

}}}

#endif /* HACKS_AUTOSTICKY_H_ */
