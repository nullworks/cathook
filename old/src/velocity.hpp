/*
 * velocity.hpp
 *
 *  Created on: May 27, 2017
 *      Author: nullifiedcat
 */

#ifndef VELOCITY_HPP_
#define VELOCITY_HPP_

#include "common.h"

namespace velocity {

typedef std::function<void(IClientEntity*, Vector&)> EstimateAbsVelocity_t;
extern EstimateAbsVelocity_t EstimateAbsVelocity;

void Init();


}

#endif /* VELOCITY_HPP_ */
