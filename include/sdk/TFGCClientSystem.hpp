/*
 * TFGCClientSystem.hpp
 *
 *  Created on: Jul 28, 2017
 *      Author: nullifiedcat
 */

#pragma once

class TFGCClientSystem {
public:
	int RequestSelectWizardStep(int type);
	int GetState();
	/* 55 89 E5 57 56 8D 75 C8 53 81 EC 8C 00 00 00 8B 45 0C C7 04 24 ? ? ? ? 8B 5D 08 89 45 A4 0F B6 C0 89 44 24 04 E8 ? ? ? ? C7 44 24 04 91 18 00 00 89 34 24 E8 ? ? ? ? A1 ? ? ? ? */
	int SendExitMatchmaking(bool abandon);
	int LoadSearchCriteria();
};

