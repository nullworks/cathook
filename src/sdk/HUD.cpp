/*
 * HUD.cpp
 *
 *  Created on: Jun 4, 2017
 *      Author: nullifiedcat
 */

#include <core/logging.hpp>
#include "sdk/HUD.h"

#include "copypasted/CSignature.h"

#include <string.h>

CHudElement *CHud::FindElement(const char *name)
{
    static uintptr_t findel_sig = gSignatures.GetClientSignature("55 89 E5 57 56 53 31 DB 83 EC 1C 8B 75 08 8B 7E 28 85 FF 7F 13 EB 49 "
                                                                 "89 F6 8D BC 27 00 00 00 00 83 C3 01 39 5E 28 7E 38 8B 46 1C 8D 3C 9D "
                                                                 "00 00 00 00 8B 04 98 8B 08 89 04 24 FF 51 24 8B 55 0C 89 04 24");
    typedef CHudElement *(*FindElement)(CHud *, const char *);
    ((FindElement)(findel_sig))(this, name);
    return ((FindElement)(findel_sig))(this, name);
}
float &CHud::GetSensitivityFactor()
{
    return *(float *) ((uintptr_t) this + 8);
}
