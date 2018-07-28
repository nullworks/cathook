/*
 * C_TEFireBullets.h
 *
 *  Created on: Jul 27, 2018
 *      Author: bencat07
 */
#pragma once
#include "reclasses.hpp"
class C_TEFireBullets : public C_BaseTempEntity
{
public:
	C_TEFireBullets() = delete;
	static C_TEFireBullets *GTEFireBullets();

public:
	int m_iSeed();
	int m_iWeaponID();
	int m_iPlayer();
	float m_flSpread();
};
