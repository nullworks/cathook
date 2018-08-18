/*
 * velocity.hpp
 *
 *  Created on: May 27, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace velocity
{

typedef std::function<void(IClientEntity *, Vector &)> EstimateAbsVelocity_t;
extern EstimateAbsVelocity_t EstimateAbsVelocity;

void Init();
} // namespace velocity