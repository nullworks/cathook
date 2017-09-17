/*
 * velocity.cpp
 *
 *  Created on: May 27, 2017
 *      Author: nullifiedcat
 */

#include "velocity.hpp"
#include "common.h"
#include "copypasted/CSignature.h"

namespace velocity {

EstimateAbsVelocity_t EstimateAbsVelocity {};

void Init() {
	EstimateAbsVelocity = (void(*)(IClientEntity*, Vector&))gSignatures.GetClientSignature("55 89 E5 56 53 83 EC 20 8B 5D 08 8B 75 0C E8 ? ? ? ? 39 D8 74 79 0F B6 05 ? ? ? ? 81 C3 B8 02 00 00 C6 05 ? ? ? ? 01 F3 0F 10 05 ? ? ? ? F3 0F 11 45 F0 88 45 EC A1 ? ? ? ? 89 45 E8 8D 45 E8 A3 ? ? ? ? A1 ? ? ? ? F3 0F 10 40 0C 89 74 24 04 89 1C 24 F3 0F 11 44 24 08 E8 ? ? ? ? 0F B6 45 EC F3 0F 10 45 F0 F3 0F 11 05 ? ? ? ? A2 ? ? ? ? 8B 45 E8 A3 ? ? ? ? 83 C4 20 5B 5E 5D C3");
}

}
