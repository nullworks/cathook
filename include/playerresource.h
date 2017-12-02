/*
 * playerresource.h
 *
 *  Created on: Nov 13, 2016
 *      Author: nullifiedcat
 */

#ifndef PLAYERRESOURCE_H_
#define PLAYERRESOURCE_H_

class CachedEntity;

class TFPlayerResource {
public:
	void Update();
	int GetMaxHealth(CachedEntity* player);
	int GetMaxBuffedHealth(CachedEntity* player);
	int GetClass(CachedEntity* player);
	int GetTeam(int idx);
	int GetScore(int idx);

	int entity;
};

extern TFPlayerResource* g_pPlayerResource;

#endif /* PLAYERRESOURCE_H_ */
