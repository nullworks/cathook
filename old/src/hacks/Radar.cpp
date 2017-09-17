/*
 * Radar.cpp
 *
 *  Created on: Mar 28, 2017
 *      Author: nullifiedcat
 */

#include "Radar.hpp"
#include "../common.h"

namespace hacks { namespace tf { namespace radar {

std::unique_ptr<textures::AtlasTexture> tx_classes[3][9];
std::unique_ptr<textures::AtlasTexture> tx_teams[2];
std::unique_ptr<textures::AtlasTexture> tx_items[2];

CatVar size(CV_INT, "radar_size", "300", "Radar size", "Defines radar size in pixels");
CatVar zoom(CV_FLOAT, "radar_zoom", "20", "Radar zoom", "Defines radar zoom (1px = Xhu)");
CatVar healthbar(CV_SWITCH, "radar_health", "1", "Radar healthbar", "Show radar healthbar");
CatVar enemies_over_teammates(CV_SWITCH, "radar_enemies_top", "1", "Show enemies on top", "If true, radar will render enemies on top of teammates");
CatVar icon_size(CV_INT, "radar_icon_size", "20", "Icon size", "Defines radar icon size");
CatVar radar_enabled(CV_SWITCH, "radar", "0", "Enable", "Enable Radar");
CatVar radar_x(CV_INT, "radar_x", "100", "Radar X", "Defines radar position (X)");
CatVar radar_y(CV_INT, "radar_y", "100", "Radar Y", "Defines radar position (Y)");
CatVar use_icons(CV_SWITCH, "radar_icons", "1", "Use Icons", "Radar will use class icons instead of class portraits");
CatVar show_teammates(CV_SWITCH, "radar_teammates", "1", "Show Teammates");
CatVar show_healthpacks(CV_SWITCH, "radar_healthpacks", "1", "Show Healthpacks");
CatVar show_ammopacks(CV_SWITCH, "radar_ammopacks", "1", "Show Ammopacks");

void Init() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			tx_classes[i][j].reset(new textures::AtlasTexture(64 * j, textures::atlas_height - 64 * (i + 1), 64, 64));
		}
	}
	tx_teams[0].reset(new textures::AtlasTexture(11 * 64, textures::atlas_height - 128, 64, 64));
	tx_teams[1].reset(new textures::AtlasTexture(11 * 64, textures::atlas_height - 64, 64, 64));

	tx_items[0].reset(new textures::AtlasTexture(10 * 64, textures::atlas_height - 64, 64, 64));
	tx_items[1].reset(new textures::AtlasTexture(10 * 64, textures::atlas_height - 128, 64, 64));
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
	int idx;
	rgba_t clr;
	float healthp;

	if (CE_GOOD(ent)) {
		if (ent->m_Type == ENTITY_PLAYER) {
			if (CE_BYTE(ent, netvar.iLifeState)) return; // DEAD. not big surprise.
			const int& clazz = CE_INT(ent, netvar.iClass);
			const int& team = CE_INT(ent, netvar.iTeamNum);
			idx = team - 2;
			if (idx < 0 || idx > 1) return;
			if (clazz <= 0 || clazz > 9) return;
			const auto& wtr = WorldToRadar(ent->m_vecOrigin.x, ent->m_vecOrigin.y);

			if (use_icons) {
				tx_teams[idx].get()->Draw(x + wtr.first, y + wtr.second, (int)icon_size, (int)icon_size);
				tx_classes[2][clazz - 1].get()->Draw(x + wtr.first, y + wtr.second, (int)icon_size, (int)icon_size);
			} else {
				tx_classes[idx][clazz - 1].get()->Draw(x + wtr.first, y + wtr.second, (int)icon_size, (int)icon_size);
				drawgl::Rect(x + wtr.first, y + wtr.second, (int)icon_size, (int)icon_size, idx ? colors::blu_v : colors::red_v);
			}

			if (ent->m_iMaxHealth && healthbar) {
				healthp = (float)ent->m_iHealth / (float)ent->m_iMaxHealth;
				clr = colors::Health(ent->m_iHealth, ent->m_iMaxHealth);
				if (healthp > 1.0f) healthp = 1.0f;
				drawgl::Rect(x + wtr.first, y + wtr.second + (int)icon_size, (int)icon_size, 4, colors::black);
				drawgl::FilledRect(x + wtr.first + 1, y + wtr.second + (int)icon_size + 1, ((float)icon_size - 2.0f) * healthp, 2, clr);
			}
		} else if (ent->m_Type == ENTITY_BUILDING) {
			/*if (ent->m_iClassID == CL_CLASS(CObjectDispenser)) {
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
		} else if (ent->m_Type == ENTITY_GENERIC) {
			if (show_healthpacks && (ent->m_ItemType == ITEM_HEALTH_LARGE || ent->m_ItemType == ITEM_HEALTH_MEDIUM || ent->m_ItemType == ITEM_HEALTH_SMALL)) {
				const auto& wtr = WorldToRadar(ent->m_vecOrigin.x, ent->m_vecOrigin.y);
				float sz = float(icon_size) * 0.15f * 0.5f;
				float sz2 = float(icon_size) * 0.85;
				tx_items[1].get()->Draw(x + wtr.first + sz, y + wtr.second + sz, sz2, sz2);
			} else if (show_ammopacks && (ent->m_ItemType == ITEM_AMMO_LARGE || ent->m_ItemType == ITEM_AMMO_MEDIUM || ent->m_ItemType == ITEM_AMMO_SMALL)) {
				const auto& wtr = WorldToRadar(ent->m_vecOrigin.x, ent->m_vecOrigin.y);
				float sz = float(icon_size) * 0.15f * 0.5f;
				float sz2 = float(icon_size) * 0.85;
				tx_items[0].get()->Draw(x + wtr.first + sz, y + wtr.second + sz, sz2, sz2);
			}
		}
	}
}

void Draw() {
	if (!g_IEngine->IsInGame()) return;
	int x, y;
	rgba_t outlineclr;
	std::vector<CachedEntity*> enemies {};
	CachedEntity *ent;

	if (!radar_enabled) return;
	x = (int)radar_x;
	y = (int)radar_y;
	int radar_size = size;
	int half_size = radar_size / 2;

	outlineclr = (hacks::shared::aimbot::foundTarget ? colors::pink : GUIColor());

	drawgl::FilledRect(x, y, radar_size, radar_size, colors::Transparent(colors::black, 0.4f));
	drawgl::Rect(x, y, radar_size, radar_size, outlineclr);

	if (enemies_over_teammates) enemies.clear();
	for (int i = 1; i < HIGHEST_ENTITY; i++) {
		ent = ENTITY(i);
		if (CE_BAD(ent)) continue;
		if (i == g_IEngine->GetLocalPlayer()) continue;
		if (ent->m_Type == ENTITY_PLAYER) {
			if (!ent->m_bEnemy && !show_teammates) continue;
		}
		if (!enemies_over_teammates || !show_teammates || ent->m_Type != ENTITY_PLAYER) DrawEntity(x, y, ent);
		else {
			if (ent->m_bEnemy) enemies.push_back(ent);
			else DrawEntity(x, y, ent);
		}
	}
	if (enemies_over_teammates && show_teammates) {
		for (auto enemy : enemies) {
			DrawEntity(x, y, enemy);
		}
	}
	if (CE_GOOD(LOCAL_E)) {
		DrawEntity(x, y, LOCAL_E);
		const auto& wtr = WorldToRadar(g_pLocalPlayer->v_Origin.x, g_pLocalPlayer->v_Origin.y);
		if (!use_icons)
			drawgl::Rect(x + wtr.first, y + wtr.second, int(icon_size), int(icon_size), GUIColor());
	}

	drawgl::Line(x + half_size, y + half_size / 2, 0, half_size, colors::Transparent(GUIColor(), 0.4f));
	drawgl::Line(x + half_size / 2, y + half_size, half_size, 0, colors::Transparent(GUIColor(), 0.4f));
}

}}}
