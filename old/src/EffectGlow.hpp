/*
 * EffectGlow.hpp
 *
 *  Created on: Apr 13, 2017
 *      Author: nullifiedcat
 */

#ifndef EFFECTGLOW_HPP_
#define EFFECTGLOW_HPP_

#include "common.h"
#include "sdk/ScreenSpaceEffects.h"
#include "sdk.h"

namespace effect_glow {

class EffectGlow : public IScreenSpaceEffect {
public:
	virtual void Init( );
	inline virtual void Shutdown( ) {};

	inline virtual void SetParameters( KeyValues *params ) {};

	virtual void Render( int x, int y, int w, int h );

	inline virtual void Enable( bool bEnable ) { enabled = bEnable; };
	inline virtual bool IsEnabled( ) { return enabled; };

	void StartStenciling();
	void EndStenciling();
	void DrawEntity(IClientEntity* entity);
	void DrawToStencil(IClientEntity* entity);
	void DrawToBuffer(IClientEntity* entity);
	rgba_t GlowColor(IClientEntity* entity);
	bool ShouldRenderGlow(IClientEntity* entity);
	void RenderGlow(IClientEntity* entity);
	void BeginRenderGlow();
	void EndRenderGlow();
public:
	bool init { false };
	bool drawing { false };
	bool enabled;
	float orig_modulation[3];
	CMaterialReference mat_blit;
	CMaterialReference mat_blur_x;
	CMaterialReference mat_blur_y;
	CMaterialReference mat_unlit;
	CMaterialReference mat_unlit_z;
};

extern EffectGlow g_EffectGlow;
extern CScreenSpaceEffectRegistration* g_pEffectGlow;

}

#endif /* EFFECTGLOW_HPP_ */
