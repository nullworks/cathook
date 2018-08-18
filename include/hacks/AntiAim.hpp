/*
 * AntiAim.h
 *
 *  Created on: Oct 26, 2016
 *      Author: nullifiedcat
 */

#pragma once

class CUserCmd;

namespace hacks::shared::antiaim
{

void SetSafeSpace(int safespace);
bool ShouldAA(CUserCmd *cmd);
void ProcessUserCmd(CUserCmd *cmd);
bool isEnabled();
} // namespace hacks::shared::antiaim
