/*
 * sdk.h
 *
 *  Created on: Dec 5, 2016
 *      Author: nullifiedcat
 */

/*
 *
 *	This source sdk was modified to suit the needs of cathook. 
 *	Please respect cathook and valves licences.
 *
 */

#ifndef SDK_H_
#define SDK_H_

//#include "fixsdk.h"

#define private public
#define protected public

#include "public/client_class.h"
#include "public/icliententity.h"
#include "public/icliententitylist.h"
#include "public/cdll_int.h"
#include "public/inetchannelinfo.h"
#include "public/gametrace.h"
#include "public/engine/IEngineTrace.h"
#include "public/materialsystem/imaterialvar.h"
#include "public/globalvars_base.h"
#include "public/materialsystem/itexture.h"
#include "public/engine/ivmodelinfo.h"
#include "public/inputsystem/iinputsystem.h"
#include "public/mathlib/vector.h"
#include "public/icvar.h"
#include "public/Color.h"
#include "public/cmodel.h"
#include "public/igameevents.h"
#include "public/iclient.h"
#include "public/inetchannel.h"
#include "public/ivrenderview.h"
#include "public/tier1/iconvar.h"
#include "public/studio.h"
#include "public/vgui/ISurface.h"
#include "public/vgui/IPanel.h"
#include "public/mathlib/vmatrix.h"
#include "public/inetmessage.h"
#include "public/iclient.h"
#include "public/iserver.h"
#include "public/view_shared.h"
#include "public/tier1/KeyValues.h"
#include "public/tier1/checksum_md5.h"
#include "public/basehandle.h"
#include "public/iachievementmgr.h"
#include "public/VGuiMatSurface/IMatSystemSurface.h"
#include "public/steam/isteamuser.h"
#include "public/steam/steam_api.h"
#include "public/vgui/Cursor.h"
#include "public/engine/ivdebugoverlay.h"
#include "public/iprediction.h"
#include "public/engine/ICollideable.h"
#include "public/tier0/icommandline.h"

#include "cathook/TFGCClientSystem.hpp"
#include "cathook/in_buttons.h"
//#include "cathook/imaterialsystemfixed.h"
#include "cathook/ScreenSpaceEffects.h"
#include "cathook/iinput.h"
#include "cathook/igamemovement.h"
//#include "cathook/HUD.h"

#endif /* SDK_H_ */
