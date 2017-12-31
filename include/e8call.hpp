/*
 * e8call.hpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <stdint.h>

inline void *e8call(void *address)
{
    return (void *) uintptr_t(*(uintptr_t *) address + uintptr_t(address) + 4);
}
