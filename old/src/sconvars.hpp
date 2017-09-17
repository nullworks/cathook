/*
 * sconvars.hpp
 *
 *  Created on: May 1, 2017
 *      Author: nullifiedcat
 */

#ifndef SCONVARS_HPP_
#define SCONVARS_HPP_

#include "common.h"

/*
 * HECK off F1ssi0N
 * I won't make NETWORK HOOKS to deal with this SHIT
 */

namespace sconvar {

class SpoofedConVar {
public:
	SpoofedConVar(ConVar* var);
public:
	ConVar* original;
	ConVar* spoof;
};

}

#endif /* SCONVARS_HPP_ */
