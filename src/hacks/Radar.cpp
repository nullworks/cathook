/*
 * Radar.cpp
 *
 *  Created on: Mar 28, 2017
 *      Author: nullifiedcat
 */

#include "Radar.hpp"
#include "../common.h"

namespace hacks { namespace tf { namespace radar {

Texture textures[2][9] = {
		{
				Texture(&_binary_scout_start, 128, 128),
				Texture(&_binary_sniper_start, 128, 128),
				Texture(&_binary_soldier_start, 128, 128),
				Texture(&_binary_demoman_start, 128, 128),
				Texture(&_binary_medic_start, 128, 128),
				Texture(&_binary_heavy_start, 128, 128),
				Texture(&_binary_pyro_start, 128, 128),
				Texture(&_binary_spy_start, 128, 128),
				Texture(&_binary_engineer_start, 128, 128)
		},
		{
				Texture(&_binary_scout_blue_start, 128, 128),
				Texture(&_binary_sniper_blue_start, 128, 128),
				Texture(&_binary_soldier_blue_start, 128, 128),
				Texture(&_binary_demoman_blue_start, 128, 128),
				Texture(&_binary_medic_blue_start, 128, 128),
				Texture(&_binary_heavy_blue_start, 128, 128),
				Texture(&_binary_pyro_blue_start, 128, 128),
				Texture(&_binary_spy_blue_start, 128, 128),
				Texture(&_binary_engineer_blue_start, 128, 128)
		}
};

Texture buildings[1] = { Texture(&_binary_dispenser_start, 128, 128) };

CatVar size(CV_INT, "radar_size", "300", "Radar size", "Defines radar size in pixels");
CatVar zoom(CV_FLOAT, "radar_zoom", "20", "Radar zoom", "Defines radar zoom (1px = Xhu)");
CatVar healthbar(CV_SWITCH, "radar_health", "1", "Radar healthbar", "Show radar healthbar");
CatVar enemies_over_teammates(CV_SWITCH, "radar_enemies_top", "1", "Show enemies on top", "If true, radar will render enemies on top of teammates");
CatVar icon_size(CV_INT, "radar_icon_size", "20", "Icon size", "Defines radar icon size");
CatVar radar_enabled(CV_SWITCH, "radar", "0", "Enable", "Enable Radar");
CatVar radar_x(CV_INT, "radar_x", "100", "Radar X", "Defines radar position (X)");
CatVar radar_y(CV_INT, "radar_y", "100", "Radar Y", "Defines radar position (Y)");

void Init() {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 9; j++) {
			textures[i][j].Load();
		}
	}
}

std::pair<int, int> WorldToRadar(int x, int y) {
	static int dx, dy, halfsize;
	static float ry, nx, ny;
	static QAngle angle;

	if (!zoom) return { 0, 0 };
	dx = x - g_pLocalPlayer->v_Origin.x;
	dy = y - g_pLocalPlayer->v_Origin.y;

	dx /= (float)zoom;
	dy /= (float)zoom;

	g_IEngine->GetViewAngles(angle);
	ry = DEG2RAD(angle.y) + PI / 2;

	dx = -dx;

	nx = dx * std::cos(ry) - dy * std::sin(ry);
	ny = dx * std::sin(ry) + dy * std::cos(ry);

	halfsize = (int)size / 2;

	if (nx < -halfsize) nx = -halfsize;
	if (nx > halfsize) nx = halfsize;
	if (ny < -halfsize) ny = -halfsize;
	if (ny > halfsize) ny = halfsize;

	return { nx + halfsize - (int)icon_size / 2, ny + halfsize - (int)icon_size / 2 };
}

void DrawEntity(int x, int y, CachedEntity* ent) {
	static int idx, clr;
	static float healthp;

	if (CE_GOOD(ent)) {
		if (ent->m_Type == ENTITY_PLAYER) {
			if (CE_BYTE(ent, netvar.iLifeState)) return; // DEAD. not big surprise.
			const int& clazz = CE_INT(ent, netvar.iClass);
			const int& team = CE_INT(ent, netvar.iTeamNum);
			idx = team - 2;
			if (idx < 0 || idx > 1) return;
			if (clazz <= 0 || clazz > 9) return;
			const auto& wtr = WorldToRadar(ent->m_vecOrigin.x, ent->m_vecOrigin.y);
			textures[idx][clazz - 1].Draw(x + wtr.first, y + wtr.second, (int)icon_size, (int)icon_size);
			draw::OutlineRect(x + wtr.first, y + wtr.second, (int)icon_size, (int)icon_size, idx ? colors::blu_v : colors::red_v);
			if (ent->m_iMaxHealth && healthbar) {
				healthp = (float)ent->m_iHealth / (float)ent->m_iMaxHealth;
				clr = colors::Health(ent->m_iHealth, ent->m_iMaxHealth);
				if (healthp > 1.0f) healthp = 1.0f;
				draw::OutlineRect(x + wtr.first, y + wtr.second + (int)icon_size, (int)icon_size, 4, colors::black);
				draw::DrawRect(x + wtr.first + 1, y + wtr.second + (int)icon_size + 1, ((float)icon_size - 2.0f) * healthp, 2, clr);
			}
		} else if (ent->m_Type == ENTITY_BUILDING) {
			/*if (ent->m_iClassID == g_pClassID->CObjectDispenser) {
				const int& team = CE_INT(ent, netvar.iTeamNum);
				int idx = team - 2;
				if (idx < 0 || idx > 1) return;
				const auto& wtr = WorldToRadar(ent->m_vecOrigin.x, ent->m_vecOrigin.y);
				buildings[0].Draw(x + wtr.first, y + wtr.second, (int)icon_size, (int)icon_size, idx ? colors::blu : colors::red	);
				draw::OutlineRect(x + wtr.first, y + wtr.second, (int)icon_size, (int)icon_size, idx ? colors::blu_v : colors::red_v);
				if (ent->m_iMaxHealth && healthbar) {
					float healthp = (float)ent->m_iHealth / (float)ent->m_iMaxHealth;
					int clr = colors::Health(ent->m_iHealth, ent->m_iMaxHealth);
					if (healthp > 1.0f) healthp = 1.0f;
					draw::OutlineRect(x + wtr.first, y + wtr.second + (int)icon_size, (int)icon_size, 4, colors::black);
					draw::DrawRect(x + wtr.first + 1, y + wtr.second + (int)icon_size + 1, ((float)icon_size - 2.0f) * healthp, 2, clr);
				}
			}*/
		}
	}
}

void Draw() {
	static int x, y, outlineclr;
	static std::vector<CachedEntity*> enemies {};
	static CachedEntity *ent;

	if (!radar_enabled) return;
	x = (int)radar_x;
	y = (int)radar_y;
	draw::DrawRect(x, y, (int)size, (int)size, colors::Transparent(colors::black, 0.4f));
	outlineclr = (hacks::shared::aimbot::state == hacks::shared::aimbot::EAimbotState::AIMING ? colors::pink : GUIColor());
	draw::OutlineRect(x, y, (int)size, (int)size, outlineclr);
	draw::DrawLine(x + (int)size / 2, y, 0, (int)size, GUIColor());
	draw::DrawLine(x, y + (int)size / 2, (int)size, 0, GUIColor());
	if (enemies_over_teammates) enemies.clear();
	for (int i = 1; i < HIGHEST_ENTITY; i++) {
		ent = ENTITY(i);
		if (CE_BAD(ent)) continue;
		if (i == g_IEngine->GetLocalPlayer()) continue;
		if (!enemies_over_teammates || ent->m_Type != ENTITY_PLAYER) DrawEntity(x, y, ent);
		else {
			if (ent->m_bEnemy) enemies.push_back(ent);
			else DrawEntity(x, y, ent);
		}
	}
	if (enemies_over_teammates) {
		for (auto enemy : enemies) {
			DrawEntity(x, y, enemy);
		}
	}
	if (CE_GOOD(LOCAL_E)) {
		DrawEntity(x, y, LOCAL_E);
		const auto& wtr = WorldToRadar(g_pLocalPlayer->v_Origin.x, g_pLocalPlayer->v_Origin.y);
		draw::OutlineRect(x + wtr.first, y + wtr.second, (int)icon_size, (int)icon_size, GUIColor());
	}
}

}}}
