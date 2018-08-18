/*
 * playerresource.h
 *
 *  Created on: Nov 13, 2016
 *      Author: nullifiedcat
 */

#pragma once

class CachedEntity;

class TFPlayerResource
{
public:
    void Update();
    int GetMaxHealth(CachedEntity *player);
    int GetMaxBuffedHealth(CachedEntity *player);
    int GetClass(CachedEntity *player);
    int GetTeam(int idx);
    int GetScore(int idx);

    int getClass(int idx);
    int getTeam(int idx);
    bool isAlive(int idx);

    int entity;
};

extern TFPlayerResource *g_pPlayerResource;
