/*
 * projlogging.cpp
 *
 *  Created on: May 26, 2017
 *      Author: nullifiedcat
 */

#include "projlogging.hpp"
#include "common.hpp"

namespace projectile_logging {

void Update() {
	for (int i = 1; i < HIGHEST_ENTITY; i++) {
		CachedEntity* ent = ENTITY(i);
		if (CE_BAD(ent)) continue;
		if (ent->m_Type == ENTITY_PROJECTILE) {
			int owner = CE_INT(ent, 0x894) & 0xFFF;
			if (owner != LOCAL_W->m_IDX) continue;
			if (tickcount % 20 == 0) {
				const Vector& v = ent->m_vecVelocity;
				const Vector& a = ent->m_vecAcceleration;
				Vector eav;
				velocity::EstimateAbsVelocity(RAW_ENT(ent), eav);
//				logging::Info("%d [%s]: CatVelocity: %.2f %.2f %.2f (%.2f) | EAV: %.2f %.2f %.2f (%.2f)", i, RAW_ENT(ent)->GetClientClass()->GetName(), v.x, v.y, v.z, v.Length(), a.x, a.y, a.z);
				logging::Info("%d [%s]: CatVelocity: %.2f %.2f %.2f (%.2f) | EAV: %.2f %.2f %.2f (%.2f)", i, RAW_ENT(ent)->GetClientClass()->GetName(), v.x, v.y, v.z, v.Length(), eav.x, eav.y, eav.z, eav.Length());
			}
		}
	}
}

}

