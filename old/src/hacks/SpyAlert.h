/*
 * SpyAlert.h
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

#ifndef HACKS_SPYALERT_H_
#define HACKS_SPYALERT_H_

#include "../common.h"

namespace hacks { namespace tf { namespace spyalert {

extern CatVar enabled;
extern CatVar distance_warning;
extern CatVar distance_backstab;

void Draw();

}}}

#endif /* HACKS_SPYALERT_H_ */
