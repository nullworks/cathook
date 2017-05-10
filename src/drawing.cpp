/*
 * drawing.cpp
 *
 *  Created on: Oct 5, 2016
 *      Author: nullifiedcat
 */

#include "common.h"
#include "sdk.h"

std::array<std::string, 32> side_strings;
std::array<std::string, 32> center_strings;
std::array<int, 32> side_strings_colors { 0 };
std::array<int, 32> center_strings_colors { 0 };
int side_strings_count { 0 };
int center_strings_count { 0 };

void InitStrings() {
	ResetStrings();
}

void ResetStrings() {
	side_strings_count = 0;
	center_strings_count = 0;
}

void AddSideString(const std::string& string, int color) {
	side_strings[side_strings_count] = string;
	side_strings_colors[side_strings_count] = color;
	side_strings_count++;
}

void DrawStrings() {
	int y { 8 };
	for (int i = 0; i < side_strings_count; i++) {
		draw::String(fonts::ESP, 8, y, side_strings_colors[i], 2, side_strings[i]);
		y += 14;
	}
	y = draw::height / 2;
	for (int i = 0; i < center_strings_count; i++) {
		draw::String(fonts::ESP, draw::width / 2, y, center_strings_colors[i], 2, center_strings[i]);
		y += 14;
	}
}

void AddCenterString(const std::string& string, int color) {
	center_strings[center_strings_count] = string;
	center_strings_colors[center_strings_count] = color;
	center_strings_count++;
}


// TODO globals
int draw::width = 0;
int draw::height = 0;

namespace colors {

int pink;

int white;
int black;

int red,    blu;
int red_b,  blu_b;  // Background
int red_v,  blu_v;  // Invis
int red_u,  blu_u;

int yellow; // Deprecated
int orange;
int green;

}

void colors::Init() {
	using namespace colors;
	pink = Create(255, 105, 180, 255);
	white = Create(255, 255, 255, 255);
	black = Create(0, 0, 0, 255);
	red = Create(237, 42, 42, 255);
	blu = Create(28, 108, 237, 255);
	red_b = Create(64, 32, 32, 178);
	blu_b = Create(32, 32, 64, 178);
	red_v = Create(196, 102, 108, 255);
	blu_v = Create(102, 182, 196, 255);
	red_u = Create(216, 34, 186, 255);
	blu_u = Create(167, 75, 252, 255);
	yellow = Create(255, 255, 0, 255);
	green = Create(0, 255, 0, 255);
	orange = Create(255, 120, 0, 255);
}

int colors::EntityF(CachedEntity* ent) {
	static int result, skin, plclr;
	static k_EItemType type;

	using namespace colors;
	result = white;
	type = ent->m_ItemType;
	if (type) {
		if (type >= ITEM_HEALTH_SMALL && type <= ITEM_HEALTH_LARGE || type == ITEM_TF2C_PILL) result = green;
		else if (type >= ITEM_POWERUP_FIRST && type <= ITEM_POWERUP_LAST) {
			skin = RAW_ENT(ent)->GetSkin();
			if (skin == 1) result = red;
			else if (skin == 2) result = blu;
			else result = yellow;
		}
		else if (type >= ITEM_TF2C_W_FIRST && type <= ITEM_TF2C_W_LAST) {
			if (CE_BYTE(ent, netvar.bRespawning)) {
				result = red;
			} else {
				result = yellow;
			}
		}
		else if (type == ITEM_HL_BATTERY) {
			result = yellow;
		}
	}

	if (ent->m_iClassID == g_pClassID->CCurrencyPack) {
		if (CE_BYTE(ent, netvar.bDistributed))
			result = red;
		else
			result = green;
	}

	if (ent->m_Type == ENTITY_PROJECTILE) {
		if (ent->m_iTeam == TEAM_BLU) result = blu;
		else if (ent->m_iTeam == TEAM_RED) result = red;
		if (ent->m_bCritProjectile) {
			if (ent->m_iTeam == TEAM_BLU) result = blu_u;
			else if (ent->m_iTeam == TEAM_RED) result = red_u;
		}
	}

	if (ent->m_Type == ENTITY_PLAYER || ent->m_Type == ENTITY_BUILDING) {
		if (ent->m_iTeam == TEAM_BLU) result = blu;
		else if (ent->m_iTeam == TEAM_RED) result = red;
		if (ent->m_Type == ENTITY_PLAYER) {
			if (IsPlayerInvulnerable(ent)) {
				if (ent->m_iTeam == TEAM_BLU) result = blu_u;
				else if (ent->m_iTeam == TEAM_RED) result = red_u;
			}
			if (HasCondition(ent, TFCond_UberBulletResist)) {
				if (ent->m_iTeam == TEAM_BLU) result = blu_v;
				else if (ent->m_iTeam == TEAM_RED) result = red_v;
			}
			plclr = playerlist::Color(ent);
			if (plclr) result = plclr;
		}
	}

	return result;
}

int colors::RainbowCurrent() {
	return colors::FromHSL(fabs(sin(g_GlobalVars->curtime / 2.0f)) * 360.0f, 0.85f, 0.9f);
}

int colors::FromHSL(float h, float s, float v) {
	static double hh, p, q, t, ff;
	static long i;

	if(s <= 0.0) {       // < is bogus, just shuts up warnings
		return colors::Create(v * 255, v * 255, v * 255, 255);
	}
	hh = h;
	if(hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = v * (1.0 - s);
	q = v * (1.0 - (s * ff));
	t = v * (1.0 - (s * (1.0 - ff)));

	switch(i) {
	case 0:
		return colors::Create(v * 255, t * 255, p * 255, 255);
	case 1:
		return colors::Create(q * 255, v * 255, p * 255, 255);
	case 2:
		return colors::Create(p * 255, v * 255, t * 255, 255);
	case 3:
		return colors::Create(p * 255, q * 255, v * 255, 255);
		break;
	case 4:
		return colors::Create(t * 255, p * 255, v * 255, 255);
	case 5:
	default:
		return colors::Create(v * 255, p * 255, q * 255, 255);
	}
}

int colors::Health(int health, int max) {
	static float hf;
	hf = (float)health / (float)max;
	if (hf > 1) {
		return colors::Create(64, 128, 255, 255);
	}
	return colors::Create((hf <= 0.5 ? 1.0 : 1.0 - 2 * (hf - 0.5)) * 255, (hf <= 0.5 ? (2 * hf) : 1) * 255, 0, 255);
}

void draw::DrawRect(int x, int y, int w, int h, int color) {
	g_ISurface->DrawSetColor(*reinterpret_cast<Color*>(&color));
	g_ISurface->DrawFilledRect(x, y, x + w, y + h);
}

namespace fonts {

unsigned long ESP = 0;
unsigned long MENU = 0;
unsigned long MENU_BIG = 0;

const std::vector<std::string> fonts = {"Tahoma Bold", "Tahoma", "TF2 Build", "Verdana", "Verdana Bold", "Arial", "Courier New", "Ubuntu Mono Bold"};
CatEnum family_enum(fonts);
CatVar esp_family(family_enum, "font_esp_family", "2", "ESP font", "ESP font family");
CatVar esp_height(CV_INT, "font_esp_height", "14", "ESP height", "ESP font height");

void Update() {
	fonts::ESP = g_ISurface->CreateFont();
	g_ISurface->SetFontGlyphSet(fonts::ESP, fonts::fonts[_clamp(0, 7, (int)fonts::esp_family)].c_str(), (int)fonts::esp_height, 0, 0, 0, 0); // or Ubuntu Mono Bold
	//g_ISurface->ResetFontCaches();
}

}

void draw::Initialize() {
	fonts::ESP = g_ISurface->CreateFont();
	fonts::MENU = g_ISurface->CreateFont();
	fonts::MENU_BIG = g_ISurface->CreateFont();
	if (!draw::width || !draw::height) {
		g_IEngine->GetScreenSize(draw::width, draw::height);
	}

	const auto install_callback_fn = [](CatVar* var) {
		var->convar_parent->InstallChangeCallback([](IConVar* var, const char* pOldValue, float flOldValue) {
			fonts::Update();
		});
	};

	fonts::esp_family.OnRegister(install_callback_fn);
	fonts::esp_height.OnRegister(install_callback_fn);

	fonts::Update();
	g_ISurface->SetFontGlyphSet(fonts::MENU, fonts::fonts[_clamp(0, 5, (int)fonts::esp_family)].c_str(), (int)fonts::esp_height, 0, 0, 0, 0);
	g_ISurface->SetFontGlyphSet(fonts::MENU, "Verdana", 12, 0, 0, 0, 0);
	g_ISurface->SetFontGlyphSet(fonts::MENU_BIG, "Verdana Bold", 30, 0, 0, 0, 0x0);
}

void draw::DrawLine(int x, int y, int dx, int dy, int color) {
	g_ISurface->DrawSetColor(*reinterpret_cast<Color*>(&color));
	g_ISurface->DrawLine(x, y, x + dx, y + dy);
}

bool draw::EntityCenterToScreen(CachedEntity* entity, Vector& out) {
	static Vector world, min, max;
	static bool succ;

	if (!entity) return false;
	RAW_ENT(entity)->GetRenderBounds(min, max);
	world = RAW_ENT(entity)->GetAbsOrigin();
	world.z += (min.z + max.z) / 2;
	succ = draw::WorldToScreen(world, out);
	return succ;
}

bool draw::WorldToScreen(Vector& origin, Vector& screen) {
	static float w, odw;

	const VMatrix& wts = g_IEngine->WorldToScreenMatrix();
	screen.z = 0;
	w = wts[3][0] * origin[0] + wts[3][1] * origin[1] + wts[3][2] * origin[2] + wts[3][3];
	if (w > 0.001) {
		odw = 1.0f / w;
		screen.x = (draw::width / 2) + (0.5 * ((wts[0][0] * origin[0] + wts[0][1] * origin[1] + wts[0][2] * origin[2] + wts[0][3]) * odw) * draw::width + 0.5);
		screen.y = (draw::height / 2) - (0.5 * ((wts[1][0] * origin[0] + wts[1][1] * origin[1] + wts[1][2] * origin[2] + wts[1][3]) * odw) * draw::height + 0.5);
		return true;
	}
	return false;
}

void draw::OutlineRect(int x, int y, int w, int h, int color) {
	g_ISurface->DrawSetColor(*reinterpret_cast<Color*>(&color));
	g_ISurface->DrawOutlinedRect(x, y, x + w, y + h);
}

void draw::GetStringLength(unsigned long font, char* string, int& length, int& height) {
	static wchar_t buf[512];
	memset(buf, 0, sizeof(wchar_t) * 512);
	mbstowcs(buf, string, strlen(string));
	g_ISurface->GetTextSize(font, buf, length, height);
}

void draw::String (unsigned long font, int x, int y, int color, int shadow, const char* text) {
	static bool newlined;
	static int w, h, s, n;
	static char ch[512];
	static wchar_t string[512];
	static size_t len;

	newlined = false;
	len = strlen(text);
	for (int i = 0; i < len; i++) {
		if (text[i] == '\n') {
			newlined = true; break;
		}
	}
	if (newlined) {
		memset(ch, 0, sizeof(char) * 512);
		GetStringLength(font, "W", w, h);
		strncpy(ch, text, 511);
		s = 0;
		n = 0;
		for (int i = 0; i < len; i++) {
			if (ch[i] == '\n') {
				ch[i] = 0;
				draw::String(font, x, y + n * (h), color, shadow, &ch[0] + s);
				n++;
				s = i + 1;
			}
		}
		draw::String(font, x, y + n * (h), color, shadow, &ch[0] + s);
	} else {
		memset(string, 0, sizeof(wchar_t) * 512);
		mbstowcs(string, text, 511);
		draw::WString(font, x, y, color, shadow, string);
	}
}

void draw::String(unsigned long font, int x, int y, int color, int shadow, std::string text) {
	draw::String(font, x, y, color, shadow, text.c_str());
}
CatVar fast_outline(CV_SWITCH, "fast_outline", "0", "Fast font outline", "Use only single repaint to increase performance");
void draw::WString(unsigned long font, int x, int y, int color, int shadow, const wchar_t* text) {
	static unsigned char alpha;
	static int black_t;

	if (shadow) {
		alpha = (color >> 24);
		black_t = ((alpha == 255) ? colors::black : colors::Create(0, 0, 0, alpha / shadow));
		if (shadow > 0) {
			draw::WString(font, x + 1, y + 1, black_t, false, text);
		}
		if (shadow > 1 && !fast_outline) {
			draw::WString(font, x - 1, y + 1, black_t, false, text);
			draw::WString(font, x - 1, y - 1, black_t, false, text);
			draw::WString(font, x + 1, y - 1, black_t, false, text);
			draw::WString(font, x + 1, y, black_t, false, text);
			draw::WString(font, x, y + 1, black_t, false, text);
			draw::WString(font, x, y - 1, black_t, false, text);
			draw::WString(font, x - 1, y, black_t, false, text);
		}
	}
	g_ISurface->DrawSetTextPos(x, y);
	g_ISurface->DrawSetTextColor(*reinterpret_cast<Color*>(&color));
	g_ISurface->DrawSetTextFont(font);
	g_ISurface->DrawUnicodeString(text);
}

void draw::FString(unsigned long font, int x, int y, int color, int shadow, const char* text, ...) {
	va_list list;
	char buffer[2048] = { '\0' };
	va_start(list, text);
	vsprintf(buffer, text, list);
	va_end(list);
	draw::String(font, x, y, color, shadow, buffer);
}

std::pair<int, int> draw::GetStringLength(unsigned long font, std::string string) {
	int l, h;
	draw::GetStringLength(font, (char*)string.c_str(), l, h);
	return std::make_pair(l, h);
}

