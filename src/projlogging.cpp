/*
 * projlogging.cpp
 *
 *  Created on: May 26, 2017
 *      Author: nullifiedcat
 */

#include "projlogging.hpp"
#include "common.hpp"

namespace projectile_logging
{
Vector prevloc[2048]{};

void Update()
{
    for (int i = 1; i <= HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        const model_t *model = RAW_ENT(ent)->GetModel();
        bool issandwich      = false;
        if (model && tickcount % 33 == 0)
        {
            std::string model_name(g_IModelInfo->GetModelName(model));
            if (model_name.find("plate") != std::string::npos)
            {
                issandwich      = true;
                Vector abs_orig = RAW_ENT(ent)->GetAbsOrigin();
                float movement  = prevloc[i].DistTo(abs_orig);
                logging::Info("Sandwich movement: %f", movement);
                prevloc[i] = abs_orig;
            }
        }
        if (ent->m_Type() == ENTITY_PROJECTILE || issandwich)
        {
            /*int owner = CE_INT(ent, 0x894) & 0xFFF;
            if (owner != LOCAL_W->m_IDX)
                continue;*/
            if (tickcount % 20 == 0)
            {
                Vector abs_orig = RAW_ENT(ent)->GetAbsOrigin();
                float movement  = prevloc[i].DistTo(abs_orig);
                logging::Info("movement: %f", movement);
                prevloc[i]      = abs_orig;
                const Vector &v = ent->m_vecVelocity;
                Vector eav;
                velocity::EstimateAbsVelocity(RAW_ENT(ent), eav);
                //				logging::Info("%d [%s]: CatVelocity: %.2f %.2f
                //%.2f
                //(%.2f) | EAV: %.2f %.2f %.2f (%.2f)", i,
                // RAW_ENT(ent)->GetClientClass()->GetName(), v.x, v.y, v.z,
                // v.Length(), a.x, a.y, a.z);
                logging::Info("%d [%s]: CatVelocity: %.2f %.2f %.2f (%.2f) | "
                              "EAV: %.2f %.2f %.2f (%.2f)",
                              i, RAW_ENT(ent)->GetClientClass()->GetName(), v.x, v.y, v.z, v.Length(), eav.x, eav.y, eav.z, eav.Length());
            }
        }
    }
}
} // namespace projectile_logging
