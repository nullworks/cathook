/*
 * ITFGroupMatchCriteria.hpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "reclasses.hpp"

namespace re
{

class ITFGroupMatchCriteria
{
public:
    enum group
    {
        CASUAL = 7
    };

public:
    static int SetMatchGroup(ITFGroupMatchCriteria *this_, int group);
};
}
