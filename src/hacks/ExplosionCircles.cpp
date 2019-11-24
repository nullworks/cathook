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

void draw_sphere(Vector center, double r, std::vector<Vector> &spherePoints)
{
    Vector screen, screen2;

    for (double phi = 0.; phi < 2 * PI; phi += PI / 10.)
    {
        spherePoints.clear();

        for (double theta = 0.; theta < PI; theta += PI / 10.)
            spherePoints.emplace_back(r * cos ( theta ) * sin ( phi ) + center.x, r * sin ( theta ) * sin ( phi ) + center.y, r * cos ( phi ) + center.z);

        spherePoints.emplace_back(center.x, center.y, center.z - r);

        int vecsize = spherePoints.size();

        Vector prev = spherePoints.front();

        for (int i = 1; i < vecsize; i++)
        {
            if (draw::WorldToScreen(prev, screen) && draw::WorldToScreen(spherePoints[i], screen2))
            {
                draw::Line(screen.x, screen.y, screen2.x - screen.x, screen2.y - screen.y, colors::FromRGBA8(30, 30, 30, 80), 2);
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
    for (int i = 0; i <= HIGHEST_ENTITY; i++)
    {
        auto ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
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
