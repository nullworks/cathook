/*
 * ITFGroupMatchCriteria.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

int re::ITFGroupMatchCriteria::SetMatchGroup(re::ITFGroupMatchCriteria *this_,
                                             int group)
{
    typedef int (*SetMatchGroup_t)(re::ITFGroupMatchCriteria *, int);
    static uintptr_t addr = gSignatures.GetClientSignature(
        "55 89 E5 56 53 83 EC 10 8B 5D 08 8B 75 0C 8B 03 89 1C 24 FF 50 08 3B "
        "70 30 74 0F 8B 03 89 1C 24 FF 50 0C 83 48 08 04 89 70 30");
    SetMatchGroup_t SetMatchGroup_fn = SetMatchGroup_t(addr);

    return SetMatchGroup_fn(this_, group);
}
