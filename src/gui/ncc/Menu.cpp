/*
 * Menu.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "../../common.h"

#include "Menu.hpp"
#include "List.hpp"
#include "ItemSublist.hpp"
#include "Tooltip.hpp"

namespace menu { namespace ncc {

unsigned long font_title = 0;
unsigned long font_item  = 0;

CatVar scale(CV_FLOAT, "gui_ncc_scale", "1", "NCC GUI Scale", "Defines scale of NCC gui", 0.5f, 4.0f);
CatVar font_family(fonts::family_enum, "gui_ncc_font_family", "3", "NCC Font Family", "Defines font family for NCC menu");
CatVar font_title_family(fonts::family_enum, "gui_ncc_font_title_family", "4", "NCC Title Family", "Defines font family for NCC menu titles");

Tooltip* tooltip = nullptr;;
Root* root = nullptr;

void ShowTooltip(const std::string& text) {
	tooltip->Show();
	tooltip->SetText(text);
}

std::vector<CatVar*> FindCatVars(const std::string name) {
	std::vector<CatVar*> result = {};
	for (auto var : CatVarList()) {
		if (var->name.find(name) == 0) result.push_back(var);
	}
	return result;
}

bool init_done = false;

void ChangeCallback(IConVar* var, const char* pszOldValue, float flOldValue) {
	if (init_done)
		RefreshFonts();
}

void Init() {
	root = new Root();
	root->Setup();
	scale.InstallChangeCallback([](IConVar* var, const char* pszOldValue, float flOldValue) {
		if (init_done) RefreshFonts();
		logging::Info("Scale Changed");
		root->HandleCustomEvent(KeyValues::AutoDelete("scale_update"));
	});
	font_family.InstallChangeCallback(ChangeCallback);
	font_title_family.InstallChangeCallback(ChangeCallback);
	init_done = true;
	RefreshFonts();
}

void RefreshFonts() {
	font_title = g_ISurface->CreateFont();
	font_item  = g_ISurface->CreateFont();
	g_ISurface->SetFontGlyphSet(font_title, fonts::fonts.at(_clamp(0, (int)(fonts::fonts.size() - 1), (int)font_title_family)).c_str(), psize_font_title * (float)scale, 0, 0, 0, 0x0);
	g_ISurface->SetFontGlyphSet(font_item, fonts::fonts.at(_clamp(0, (int)(fonts::fonts.size() - 1), (int)font_family)).c_str(), psize_font_item * (float)scale, 0, 0, 0, 0x0);
	root->HandleCustomEvent(KeyValues::AutoDelete("font_update"));
}

static const std::string list_hl2dm = R"(
"Cat Hook"
"Aim Bot" [
	"Aim Bot Menu"
	"aimbot_enabled"
	"aimbot_aimkey"
	"aimbot_aimkey_mode"
	"aimbot_autoshoot"
	"aimbot_hitboxmode"
	"aimbot_fov"
	"aimbot_prioritymode"
	"aimbot_charge"
    "aimbot_teammates"
	"Preferences" [
		"Aim Bot Preferences"
		"aimbot_silent"
		"aimbot_hitbox"
		"aimbot_only_when_can_shoot"
		"aimbot_enable_attack_only"
		"aimbot_maxrange"
		"aimbot_interp"
		"aimbot_slow"
		"aimbot_slow_smooth"
		"aimbot_slow_autoshoot"
	]
]
 
"Trigger Bot" [
	"Trigger Bot Menu"
	"trigger_enabled"
	"trigger_accuracy"
	"trigger_range"
	"trigger_hitbox"
]
 


"Visuals" [
	"Visuals Menu"
	"ESP" [
		"ESP Menu"
		"esp_enabled"
		"font_esp_family"
		"font_esp_height"
		"esp_name"
		"esp_distance"
		"esp_box"
		"esp_box_text_position"
		"esp_box_nodraw"
		"esp_box_expand"
		"3D Box" [
			"3D Box Settings"
			"esp_3d_box"
			"esp_3d_box_thick"
			"esp_3d_box_health"
			"esp_3d_box_expand"
			"esp_3d_box_smoothing"
			"esp_3d_box_expand_size"
			"esp_3d_box_healthbar"
		]
		"esp_legit"
		"esp_health_num"
		"esp_weapon_spawners"
		"esp_model_name"
		"esp_weapon"
		"esp_vischeck"
		"esp_entity_id"
		"esp_followbot_id"
		"esp_teammates"
		"esp_entity"
		"esp_local"
		"Items" [
			"Item ESP Menu"
			"esp_item"
			"esp_item_adrenaline"
			"esp_item_powerups"
			"esp_item_health"
			"esp_item_ammo"
			"esp_item_weapons"
		]
	]
	"Chams" [
		"Chams Menu"
		"chams_enable"
		"chams_health"
		"chams_players"
		"chams_teammates"
		"chams_flat"
		"chams_weapons"
		"chams_medkits"
		"chams_ammo"
	]

	"Glow" [
		"Glow Menu"
		"glow_enable"
		"glow_solid_when"
		"glow_blur_scale"
		"glow_health"
		"glow_players"
		"glow_teammates"
		"glow_medkits"
		"glow_ammo"
	]
	"GUI" [
		"GUI Settings"
		"logo"
		"gui_bg_particles"
		"gui_bg_particles_type"
		"gui_bg_particles_chance"
		"gui_bg_particles_pack_size"
		"gui_bg_particles_safe_zone"
		"gui_bg_particles_gravity"
		"gui_bg_particles_jittering"
		"gui_bg_particles_jittering_chance"
		"gui_bg_particles_wind"
		"gui_bg_visible"
		"gui_color_b"
		"gui_color_g"
		"gui_color_r"
		"gui_rainbow"
		"fast_outline"
		"gui_bounds"
		"gui_visible"
	]
	"Miscellaneous" [
		"Miscellaneous"
		"pure_bypass"
		"no_arms"
		"fov"
		"clean_screenshots"
		"logo"
	]
]
 
"Anti-/Anti-Aim" [
	"Anti-Aim Menu"
	"aa_enabled"
	"aa_pitch"
	"aa_pitch_mode"
	"aa_yaw"
	"aa_yaw_mode"
	"aa_spin"
	"aa_roll"
	"aa_no_clamp"
	"resolver"
	"Anti-Anti-Anti-Aim" [
		"Anti-Anti-Anti-Aim Menu"
		"aa_aaaa_enabled"
		"aa_aaaa_interval"
		"aa_aaaa_interval_low"
		"aa_aaaa_interval_high"
		"aa_aaaa_mode"
		"aa_aaaa_flip_key"
	]
]

"Chat" [
	"Chat Options Menu"
	"chat_newlines"
	"clean_chat"
	"killsay"
	"spam"
	"spam_random"
]
 
"Follow Bot" [
	"Follow Bot Settings"
	"fb_bot"
	"fb_mimic_slot"
	"fb_always_medigun"
	"fb_autoclass"
	"fb_follow_distance"
]

"Miscellaneous" [
	"Miscellaneous Settings"
	"bhop_enabled"
	"fast_vischeck"
	"anti_afk"
	"rollspeedhack"
	"info"
	"Debug" [
		"Debug Menu"
		"software_cursor_mode"
		"enabled"
		"no_visuals"
		"debug_info"
		"log"
	]
]
)";

static const std::string list_tf2 = R"(
"Cat Hook"
"Aim Bot" [
    "Aim Bot Menu"
    "aimbot_enabled"
    "aimbot_aimkey"
    "aimbot_aimkey_mode"
    "aimbot_autoshoot"
    "aimbot_hitboxmode"
    "aimbot_fov"
    "aimbot_prioritymode"
    "aimbot_charge"
    "Ignore" [
        "Ignore/Respect Menu"
        "aimbot_respect_vaccinator"
        "ignore_taunting"
        "aimbot_ignore_hoovy"
        "aimbot_respect_cloak"
        "aimbot_buildings"
        "aimbot_teammates"
    ]
    "Auto Heal" [
        "Auto Heal Menu"
        "autoheal_enabled"
        "autoheal_uber"
        "autoheal_uber_health"
        "autoheal_silent"
        "autoheal_share_uber"
    ]
    "Preferences" [
        "Aim Bot Preferences"
        "aimbot_silent"
        "aimbot_hitbox"
        "aimbot_zoomed"
        "aimbot_only_when_can_shoot"
        "aimbot_enable_attack_only"
        "aimbot_maxrange"
        "aimbot_interp"
        "aimbot_instant_rezoom_enabled"
        "aimbot_slow"
        "aimbot_slow_smooth"
        "aimbot_slow_autoshoot"
        "Projectile Aimbot" [
            "Projectile Aimbot Tweaks"
            "aimbot_projectile"
            "aimbot_proj_fovpred"
            "aimbot_proj_vispred"
            "aimbot_proj_gravity"
            "aimbot_proj_speed"
            "aimbot_huntsman_charge"
            "aimbot_full_auto_huntsman"
        ]
    ]
]
 
"Trigger Bot" [
    "Trigger Bot Menu"
    "trigger_enabled"
    "autobackstab"
    "Auto Vaccinator" [
        "Auto Vaccinator"
        "auto_vacc"
        "auto_vacc_reset_timer"
        "auto_vacc_default_resist"
        "auto_vacc_bullets"
        "auto_vacc_blast"
        "auto_vacc_fire"
        "auto_vacc_sniper_pop"
        "auto_vacc_blast_pop_health"
        "auto_vacc_blast_pop_crit"
        "auto_vacc_rocket_range"
        "auto_vacc_fire_pop_pyro"
        "auto_vacc_afterburn"
        "auto_vacc_pyro_range"
        "auto_vacc_bullet_pop_ubers"
        "auto_vacc_blast_pop_ubers"
        "auto_vacc_fire_pop_ubers"
    ]
    "Auto Sticky" [
        "Auto Sticky Menu"
        "sticky_enabled"
        "sticky_distance"
        "sticky_buildings"
        "sticky_visable"
    ]
    "Auto Reflect" [
        "Auto Reflect Menu"
        "reflect_enabled"
        "reflect_distance"
        "reflect_stickybombs"
        "reflect_only_idle"
    ]
    "Triggerbot Ignores" [
        "Ignore/Respect Menu"
        "trigger_respect_vaccinator"
        "trigger_respect_cloak"
        "trigger_buildings"
    ]
    "Triggerbot Preferences" [
        "Triggerbot Preferences"
        "trigger_accuracy"
        "trigger_ambassador"
        "trigger_range"
        "trigger_finish"
        "trigger_bodyshot"
        "trigger_hitbox"
        "trigger_zoomed"
    ]
]
 


"Visuals" [
    "Visuals Menu"
    "ESP" [
        "ESP Menu"
        "esp_enabled"
        "font_esp_family"
        "font_esp_height"
        "esp_conds"
        "esp_class"
        "esp_name"
        "esp_distance"
        "esp_box"
        "esp_box_text_position"
        "esp_box_nodraw"
        "esp_box_expand"
        "3D Box" [
            "3D Box Settings"
            "esp_3d_box"
            "esp_3d_box_thick"
            "esp_3d_box_health"
            "esp_3d_box_expand"
            "esp_3d_box_smoothing"
            "esp_3d_box_expand_size"
            "esp_3d_box_healthbar"
        ]
        "esp_legit"
        "esp_health_num"
        "esp_weapon_spawners"
        "esp_model_name"
        "esp_weapon"
        "esp_vischeck"
        "esp_show_tank"
        "esp_entity_id"
        "esp_followbot_id"
        "esp_teammates"
        "esp_entity"
        "esp_buildings"
        "esp_local"
        "Items" [
            "Item ESP Menu"
            "esp_item"
            "esp_item_adrenaline"
            "esp_item_powerups"
            "esp_item_health"
            "esp_item_ammo"
            "esp_item_weapons"
            "esp_money_red"
            "esp_money"
        ]
        "Projectiles" [
            "Projectile ESP Menu"
            "esp_proj"
            "esp_proj_enemy"
            "esp_proj_stickies"
            "esp_proj_pipes"
            "esp_proj_arrows"
            "esp_proj_rockets"
        ]
    ]
    "Radar" [
        "Radar Menu"
        "radar"
        "radar_size"
        "radar_zoom"
        "radar_health"
        "radar_enemies_top"
        "radar_icon_size"
        "radar_x"
        "radar_y"
    ]
    "Chams" [
        "Chams Menu"
        "chams_enable"
        "chams_health"
        "chams_players"
        "chams_teammates"
        "chams_buildings"
        "chams_teammate_buildings"
        "chams_flat"
        "chams_weapons"
        "chams_medkits"
        "chams_ammo"
        "chams_stickies"
    ]

    "Glow" [
        "Glow Menu"
        "glow_enable"
        "glow_solid_when"
        "glow_blur_scale"
        "glow_health"
        "glow_players"
        "glow_teammates"
        "glow_buildings"
        "glow_teammate_buildings"
        "glow_medkits"
        "glow_ammo"
        "glow_stickies"
    ]

    "TF2 Glow Outline" [
        "TF2 Glow Menu"
        "glow_old_enabled"
        "glow_old_players"
        "glow_old_color_scheme"
        "glow_old_health_packs"
        "glow_old_ammo_boxes"
        "glow_old_alpha"
        "glow_old_teammates"
        "glow_old_teammate_buildings"
        "glow_old_buildings"
        "glow_old_stickies"
    ]    
    "GUI" [
        "GUI Settings"
        "logo"
        "gui_bg_particles"
        "gui_bg_particles_type"
        "gui_bg_particles_chance"
        "gui_bg_particles_pack_size"
        "gui_bg_particles_safe_zone"
        "gui_bg_particles_gravity"
        "gui_bg_particles_jittering"
        "gui_bg_particles_jittering_chance"
        "gui_bg_particles_wind"
        "gui_bg_visible"
        "gui_color_b"
        "gui_color_g"
        "gui_color_r"
        "gui_rainbow"
        "fast_outline"
        "gui_bounds"
        "gui_visible"
    ]
    "Miscellaneous" [
        "Miscellaneous"
        "pure_bypass"
        "antidisguise"
        "no_arms"
        "no_hats"
        "thirdperson"
        "thirdperson_angles"
        "render_zoomed"
        "fov"
        "fov_zoomed"
        "no_zoom"
        "clean_screenshots"
        "logo"
    ]
]
 
"Anti-/Anti-Aim" [
    "Anti-Aim Menu"
    "aa_enabled"
    "aa_pitch"
    "aa_pitch_mode"
    "aa_yaw"
    "aa_yaw_mode"
    "aa_spin"
    "aa_roll"
    "aa_no_clamp"
    "resolver"
    "Anti-Anti-Anti-Aim" [
        "Anti-Anti-Anti-Aim Menu"
        "aa_aaaa_enabled"
        "aa_aaaa_interval"
        "aa_aaaa_interval_low"
        "aa_aaaa_interval_high"
        "aa_aaaa_mode"
        "aa_aaaa_flip_key"
    ]
]
 
"Crit Hack" [
    "Crit Hack Menu"
    "crit_info"
    "crit_hack_next"
    "crit_hack"
    "crit_suppress"
    "crit_melee"
    "crit_hack_experimental"
]

"Chat" [
    "Chat Options Menu"
    "chat_newlines"
    "clean_chat"
    "killsay"
    "spam"
    "spam_random"
    "uberspam"
    "uberspam_build"
    "uberspam_ready"
    "uberspam_used"
    "uberspam_ended"
    "uberspam_team"
]
 
"Follow Bot" [
    "Follow Bot Settings"
    "fb_bot"
    "fb_mimic_slot"
    "fb_always_medigun"
    "fb_autoclass"
    "fb_follow_distance"
]

"Miscellaneous" [
    "Miscellaneous Settings"
    "removecond_key"
    "removecond_fire"
    "removecond_taunt"
    "removecond_bleeding"
    "removecond_stun"
    "spycrab"
    "Spy Alert" [
        "Spy Alert Settings"
        "spyalert_enabled"
        "spyalert_warning"
        "spyalert_backstab"
        "spyalert_sound"
        "spyalert_interval"
    ]
    "Anti Backstab" [
        "Anti Backstab Menu"
        "antibackstab"
        "antibackstab_nope"
        "antibackstab_angle"
        "antibackstab_distance"
        "antibackstab_silent"
    ]
    "bhop_enabled"
    "noisemaker"
    "nopush_enabled"
    "fast_vischeck"
    "anti_afk"
    "rollspeedhack"
    "info"
    "Debug" [
        "Debug Menu"
        "software_cursor_mode"
        "enabled"
        "no_visuals"
        "debug_info"
        "log"
    ]
]

)");
	return *main;
}

}}