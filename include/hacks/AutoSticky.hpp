/*
 * AutoSticky.h
 *
 *  Created on: Dec 2, 2016
 *      Author: nullifiedcat
 */

#ifndef HACKS_AUTOSTICKY_HPP_
#define HACKS_AUTOSTICKY_HPP_

#include "common.hpp"

namespace hacks
{
namespace tf
{
namespace autosticky
{

extern CatVar enabled;
extern CatVar buildings;
extern CatVar distance;

bool ShouldDetonate(CachedEntity *bomb);
void CreateMove();
}
}
}

#endif /* HACKS_AUTOSTICKY_HPP_ */
