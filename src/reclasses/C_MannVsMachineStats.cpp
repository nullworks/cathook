/*
 * CMannVsMachineStats.cpp
 *
 *  Created on: Apr 10, 2018
 *      Author: bencat07
 */
#include "common.hpp"
using namespace re;

// Unsued right now because of unclear values in AddLocalPlayerUpgrade
C_MannVsMachineStats *C_MannVsMachineStats::G_MannVsMachineStats()
{
    typedef C_MannVsMachineStats *(*G_MannVsMachineStats_t)();
    static uintptr_t addr = gSignatures.GetClientSignature(
        XORSTR("55 A1 ? ? ? ? 89 E5 5D C3 8D B6 00 00 00 00 55 89 E5 53 83 EC ? 8B 5D "
        "? C7 83 ? ? ? ? ? ? ? ?"));
    static G_MannVsMachineStats_t G_MannVsMachineStats_fn =
        G_MannVsMachineStats_t(addr);

    return G_MannVsMachineStats_fn();
}
int *C_MannVsMachineStats::AddLocalPlayerUpgrade(int id, int &a3)
{
    typedef int *(*AddLocalPlayerUpgrade_t)(C_MannVsMachineStats *, int, int);
    static uintptr_t addr = gSignatures.GetClientSignature(
        XORSTR("55 89 E5 57 56 53 83 EC ? 8B 5D ? 8B 75 ? 8B 7D ? 8B 43 ? 8B 53 ? 83 "
        "C0 ? 39 D0 7E ? 29 D0 89 1C 24 89 44 24 ? E8 ? ? ? ? 8B 43 ? 83 C0 ? "
        "8B 13 89 43 ? 29 F0 83 E8 ? 89 F1 C1 E1 ?"));
    static AddLocalPlayerUpgrade_t AddLocalPlayerUpgrade_fn =
        AddLocalPlayerUpgrade_t(addr);

    return AddLocalPlayerUpgrade_fn(this, id, a3);
}
