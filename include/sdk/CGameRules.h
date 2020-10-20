/*
 * CGameRules.h
 *
 *  Created on: Nov 23, 2017
 *      Author: nullifiedcat
 */

#pragma once
class CGameRules
{
public:
    int pad0[12];     // 0    | 48 bytes  | 48
    int roundmode;    // 48   | 4 bytes   | 52
    int pad1[1];      // 52   | 4 bytes   | 56
    int winning_team; // 56   | 4 bytes   | 60
};
