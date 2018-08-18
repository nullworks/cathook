/*
 * CTFParty.hpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "reclasses.hpp"

namespace re
{

class CTFParty
{
public:
    // 7 Dec client.so:sub_B986E0
    static CTFParty *GetParty();

    inline static int state_(CTFParty *party)
    {
        return *((int *) party + 19);
    }
};
} // namespace re
