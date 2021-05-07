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
    int pad0[12];               // 0    | 48 bytes  | 48
    int roundmode;              // 48   | 4 bytes   | 52
    int m_bInWaitingForPlayers; // 52   | 4 bytes   | 56
    int winning_team;           // 56   | 4 bytes   | 60
    char pad2[974];             // 60   | 974 bytes | 1034
    bool isPVEMode;             // 1034 | 1 byte    | 1035
};
