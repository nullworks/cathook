/*
 * AntiAim.h
 *
 *  Created on: Oct 26, 2016
 *      Author: nullifiedcat
 */

#ifndef ANTIAIM_H_
#define ANTIAIM_H_

class CatVar;
class CUserCmd;

namespace hacks { namespace shared { namespace antiaim {

// TODO paste AA from AimTux

extern CatVar enabled;
extern CatVar yaw;
extern CatVar pitch;
extern CatVar yaw_mode;
extern CatVar pitch_mode;
extern CatVar roll;
extern CatVar no_clamping;
extern CatVar spin;
extern CatVar lisp;

extern int safe_space;

void SetSafeSpace(int safespace);
bool ShouldAA(CUserCmd* cmd);
void ProcessUserCmd(CUserCmd* cmd);

}}}

#endif /* ANTIAIM_H_ */
