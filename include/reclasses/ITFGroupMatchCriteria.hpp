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
        MvmPractice                = 0,
        MvmMannup                  = 1,
        LadderMatch6v6             = 2,
        LadderMatch9v9             = 3,
        LadderMatch12v12           = 4,
        CasualMatch6v6             = 5,
        CasualMatch9v9             = 6,
        CasualMatch12v12           = 7,
        CompetitiveEventMatch12v12 = 8
    };

public:
    static int SetMatchGroup(ITFGroupMatchCriteria *this_, int group);
};
} // namespace re
