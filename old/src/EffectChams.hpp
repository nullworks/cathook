/*
 * EffectChams.hpp
 *
 *  Created on: Apr 16, 2017
 *      Author: nullifiedcat
 */

#ifndef EFFECTCHAMS_HPP_
#define EFFECTCHAMS_HPP_

#include "common.h"

namespace effect_chams {

class EffectChams : public IScreenSpaceEffect {
public:
	virtual void Init( );
	inline virtual void Shutdown( ) {};

	inline virtual void SetParameters( KeyValues *params ) {};

	virtual void Render( int x, int y, int w, int h );

	inline virtual void Enable( bool bEnable ) { enabled = bEnable; };
	inline virtual bool IsEnabled( ) { return enabled; };

	rgba_t ChamsColor(IClientEntity* entity);
	bool ShouldRenderChams(IClientEntity* entity);
	void RenderChams(int idx);
	void BeginRenderChams();
	void EndRenderChams();
	void RenderChamsRecursive(IClientEntity* entity);
public:
	bool init { false };
	bool drawing { false };
	bool enabled;
	float orig_modulation[3];
	CMaterialReference mat_unlit;
	CMaterialReference mat_unlit_z;
	CMaterialReference mat_lit;
	CMaterialReference mat_lit_z;
};

extern EffectChams g_EffectChams;
extern CScreenSpaceEffectRegistration* g_pEffectChams;

}

#endif /* EFFECTCHAMS_HPP_ */
