/*  This file is part of Cathook.

    Cathook is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cathook is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cathook.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "common.hpp"
#include "drawing.hpp"

namespace hacks::tf2::explosioncircles
{
static settings::Boolean enabled("explosionspheres.enabled", "false");

// too lazy to make my own http://www.cplusplus.com/forum/general/65476/
void draw_sphere(Vector center, double r, std::vector<Vector> &spherePoints)
{
    Vector screen, screen2;
    // Iterate through phi, theta then convert r,theta,phi to  XYZ
    for (double phi = 0.; phi < 2 * PI; phi += PI / 10.) // Azimuth [0, 2PI]
    {
        spherePoints.clear();
        for (double theta = 0.; theta < PI; theta += PI / 10.) // Elevation [0, PI]
            spherePoints.emplace_back(r * cos(phi) * sin(theta) + center.x, r * sin(phi) * sin(theta) + center.y, r * cos(theta) + center.z);
        // Add the missing point on the bottom
        spherePoints.emplace_back(center.x, center.y, center.z - r);
        int vecsize = spherePoints.size();
        Vector prev = spherePoints.front();
        for (int i = 1; i < vecsize; ++i)
        {
            if (draw::WorldToScreen(prev, screen) && draw::WorldToScreen(spherePoints[i], screen2))
            {
                draw::Line(screen.x, screen.y, screen2.x - screen.x, screen2.y - screen.y, colors::FromRGBA8(255, 120, 0, 50), 2);
            }
            prev = spherePoints[i];
        }
    }
    return;
}

void draw()
{
    if (!enabled)
        return;
    if (!CE_GOOD(LOCAL_E))
        return;
    std::vector<Vector> points;
    Vector screen;
    for (auto const &ent : entity_cache::valid_ents)
    {
        if (!ent->m_bEnemy())
            continue;
        if (ent->m_iClassID() != CL_CLASS(CTFGrenadePipebombProjectile))
            continue;
        if (CE_INT(ent, netvar.iPipeType) != 1)
            continue;
        if (!draw::WorldToScreen(ent->m_vecOrigin(), screen))
            continue;
        draw_sphere(ent->m_vecOrigin(), CE_FLOAT(ent, netvar.m_DmgRadius), points);
    }
}

static InitRoutine init([]() { EC::Register(EC::Draw, draw, "draw_explosioncircles"); });
} // namespace hacks::tf2::explosioncircles
