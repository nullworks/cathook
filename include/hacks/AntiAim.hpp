/*
 * AntiAim.h
 *
 *  Created on: Oct 26, 2016
 *      Author: nullifiedcat
 */

#pragma once
#include <inetmessage.h>

class CUserCmd;

namespace hacks::shared::antiaim
{
extern bool force_fakelag;
extern float used_yaw;
void SetSafeSpace(int safespace);
bool ShouldAA(CUserCmd *cmd);
void ProcessUserCmd(CUserCmd *cmd);
bool isEnabled();
void SendNetMessage(INetMessage &msg);
} // namespace hacks::shared::antiaim
