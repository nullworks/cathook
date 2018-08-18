/*
 * C_TEFireBullets.cpp
 *
 *  Created on: Jul 27, 2018
 *      Author: bencat07
 */
#include "reclasses.hpp"
#pragma once

C_BaseTempEntity *C_TEFireBullets::GTEFireBullets()
{
    static uintptr_t fireaddr =
        (gSignatures.GetClientSignature(
             "C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? A3 ? ? ? ? 89 50") -
         0x4);
    C_BaseTempEntity *fire = *(C_BaseTempEntity **) fireaddr;
    return fire;
}
int C_TEFireBullets::m_iSeed()
{
    return int(unsigned(this) + 0x34);
}
int C_TEFireBullets::m_iWeaponID()
{
    return int(unsigned(this) + 0x2c);
}
int C_TEFireBullets::m_iPlayer()
{
    return int(unsigned(this) + 0x10);
}
float C_TEFireBullets::m_flSpread()
{
    return float(unsigned(this) + 0x38);
}
