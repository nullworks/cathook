/*
 * CTFParty.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "core/e8call.hpp"

re::CTFParty *re::CTFParty::GetParty()
{
    typedef re::CTFParty *(*GetParty_t)(void *);
    typedef void *(*GTFGCClientSystem_t)();

    static uintptr_t addr1 = gSignatures.GetClientSignature(
        XORSTR("E8 ? ? ? ? 84 C0 0F 85 7B 02 00 00 E8 ? ? ? ? BE 01 00 00 00 89 04 24 "
        "E8 ? ? ? ? 85 C0 0F 84 E5 02 00 00"));

    static GTFGCClientSystem_t GTFGCClientSystem_fn =
        GTFGCClientSystem_t(e8call((void *) (addr1 + 14)));
    static GetParty_t GetParty_fn = GetParty_t(e8call((void *) (addr1 + 27)));

    return GetParty_fn(GTFGCClientSystem_fn());
}
