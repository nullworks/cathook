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
boost::unordered_flat_map<u_int16_t, Vector> prevloc;

void Update()
{
    for (auto const &ent : entity_cache::valid_ents)
    {

        const model_t *model    = RAW_ENT(ent)->GetModel();
        bool issandwich         = false;
        const uint16_t curr_idx = ent->m_IDX;
        if (model && tickcount % 33 == 0)
        {
            std::string model_name(g_IModelInfo->GetModelName(model));
            if (model_name.find("plate") != std::string::npos)
            {
                issandwich      = true;
                Vector abs_orig = RAW_ENT(ent)->GetAbsOrigin();
                float movement  = prevloc[curr_idx].DistTo(abs_orig);
                logging::Info("Sandwich movement: %f", movement);
                prevloc[curr_idx] = abs_orig;
            }
        }
        if (ent->m_Type() == ENTITY_PROJECTILE || issandwich)
        {
            /*int owner = HandleToIDX(CE_INT(ent, 0x894));
            if (owner != LOCAL_W->m_IDX)
                continue;*/
            if (tickcount % 20 == 0)
            {
                Vector abs_orig = RAW_ENT(ent)->GetAbsOrigin();
                float movement  = prevloc[curr_idx].DistTo(abs_orig);
                logging::Info("movement: %f", movement);
                prevloc[curr_idx] = abs_orig;
                const Vector &v   = ent->m_vecVelocity;
                Vector eav;
                velocity::EstimateAbsVelocity(RAW_ENT(ent), eav);
                //				logging::Info("%d [%s]: CatVelocity: %.2f %.2f
                //%.2f
                //(%.2f) | EAV: %.2f %.2f %.2f (%.2f)", i,
                // RAW_ENT(ent)->GetClientClass()->GetName(), v.x, v.y, v.z,
                // v.Length(), a.x, a.y, a.z);
                logging::Info("%d [%s]: CatVelocity: %.2f %.2f %.2f (%.2f) | "
                              "EAV: %.2f %.2f %.2f (%.2f)",
                              curr_idx, RAW_ENT(ent)->GetClientClass()->GetName(), v.x, v.y, v.z, v.Length(), eav.x, eav.y, eav.z, eav.Length());
            }
        }
    }
}
} // namespace projectile_logging
