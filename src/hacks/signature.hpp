
/*
 *
 *	Header for the signatures file.
 *	Include this to make use of signatures!
 *
 *	Copypasted from Darkstorm 2015 linux base
 *
 */

#pragma once

#include <stdint.h>

#include "sharedobj.hpp"

namespace hacks {

uintptr_t GetObjectSignature(const SharedObject& shared_obj, const char* chPattern);

}
