/*
 * TFGCClientSystem.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: nullifiedcat
 */

#include "copypasted/CSignature.h"
#include "sdk/TFGCClientSystem.hpp"

#include <logging.hpp>


int TFGCClientSystem::RequestSelectWizardStep(int type) {
	static uintptr_t RequestSelectWizardStep_loc = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC 1C 8B 5D 08 8B 75 0C 89 1C 24 E8 ? ? ? ? 84 C0 75 0D 83 C4 1C 5B 5E 5F 5D C3 90 8D 74 26 00 89 1C 24 E8 ? ? ? ? 85 C0");
	typedef int(*RequestSelectWizardStep_t)(TFGCClientSystem*, int);
	static RequestSelectWizardStep_t RequestSelectWizardStep_fn = (RequestSelectWizardStep_t)RequestSelectWizardStep_loc;
	return RequestSelectWizardStep_fn(this, 4);
}

int TFGCClientSystem::GetState() {
	return *(int*)((uintptr_t)this + 1136);
}

/* 55 89 E5 57 56 8D 75 C8 53 81 EC 8C 00 00 00 8B 45 0C C7 04 24 ? ? ? ? 8B 5D 08 89 45 A4 0F B6 C0 89 44 24 04 E8 ? ? ? ? C7 44 24 04 91 18 00 00 89 34 24 E8 ? ? ? ? A1 ? ? ? ? */
int TFGCClientSystem::SendExitMatchmaking(bool abandon) {
	static uintptr_t SendExitMatchmaking_loc = gSignatures.GetClientSignature("55 89 E5 57 56 8D 75 C8 53 81 EC 8C 00 00 00 8B 45 0C C7 04 24 ? ? ? ? 8B 5D 08 89 45 A4 0F B6 C0 89 44");
	typedef int(*SendExitMatchmaking_t)(TFGCClientSystem*, char);
	static SendExitMatchmaking_t SendExitMatchmaking_fn = (SendExitMatchmaking_t)SendExitMatchmaking_loc;
	logging::Info("Calling 0x%08x", SendExitMatchmaking_fn);
	return SendExitMatchmaking_fn(this, abandon);
}

int TFGCClientSystem::LoadSearchCriteria() {
	static uintptr_t LoadSearchCriteria_loc = gSignatures.GetClientSignature("55 89 E5 57 56 53 8D 5D A8 81 EC DC 00 00 00 C7 44 24 0C 00 00 00 00 C7 44 24 08 00 00 00 00 C7 44 24 04 00 00 00 00 89 1C 24 E8 ? ? ? ? C7 44 24 08 01 00 00 00 C7 44 24 04 01 00 00 00 89 1C 24 E8 ? ? ? ? A1 ? ? ? ? 8D 50 04");
	typedef int(*LoadSearchCriteria_t)(TFGCClientSystem*);
	static LoadSearchCriteria_t LoadSearchCriteria_fn = (LoadSearchCriteria_t)LoadSearchCriteria_loc;
	logging::Info("Calling 0x%08x", LoadSearchCriteria_fn);
	return LoadSearchCriteria_fn(this);
}
