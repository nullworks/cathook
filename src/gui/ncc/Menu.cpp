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

Tooltip* tooltip = 0;

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

void Init() {
	font_title = g_ISurface->CreateFont();
	font_item  = g_ISurface->CreateFont();
	g_ISurface->SetFontGlyphSet(font_title, "Verdana Bold", 14, 0, 0, 0, 0x0);
	g_ISurface->SetFontGlyphSet(font_item, "Verdana", 12, 0, 0, 0, 0x0);
	// TODO add stuff
	/*List* aimbot_list = new List("Aim Bot Menu");
	aimbot_list->FillWithCatVars({
		"aimbot_enabled", "aimbot_fov", "aimbot_aimkey", "aimbot_aimkey_mode", "aimbot_hitboxmode", "aimbot_hitbox",
		"aimbot_silent", "aimbot_prioritymode", "aimbot_autoshoot", "aimbot_zoomed", "aimbot_teammates", "aimbot_buildings", "aimbot_respect_cloak",
		"aimbot_projectile"
	});
	List* aimbot_list_projectile = new List("Projectile Aimbot");
	aimbot_list_projectile->FillWithCatVars({
		"aimbot_huntsman_charge", "aimbot_full_auto_huntsman",
		"aimbot_proj_fovpred", "aimbot_proj_vispred", "aimbot_proj_gravity", "aimbot_proj_speed"
	});
	aimbot_list->AddChild(new ItemSublist("Projectile Aimbot Tweaks", aimbot_list_projectile));
	aimbot_list->FillWithCatVars({
		"aimbot_only_when_can_shoot", "aimbot_enable_attack_only", "aimbot_maxrange", "aimbot_interp"
	});
	//aimbot_list->FillWithCatVars(FindCatVars("aimbot_"));
	MainList().AddChild(new ItemSublist("Aim Bot", aimbot_list));
	List* triggerbot_list = new List("Trigger Bot Menu");
	triggerbot_list->FillWithCatVars(FindCatVars("trigger_"));
	MainList().AddChild(new ItemSublist("Trigger Bot", triggerbot_list));
	//MainList().AddChild(new ItemSublist("Accuracy", nullptr));
	// Fake Lag

	List* esp_list = new List("ESP Menu");
	esp_list->FillWithCatVars(FindCatVars("esp_"));
	MainList().AddChild(new ItemSublist("ESP", esp_list));
	MainList().Show();
	// Radar
	/*MainList().AddChild(new ItemSublist("Bunny Hop", nullptr));
	MainList().AddChild(new ItemSublist("Air Stuck", nullptr));
	// Speed Hack
	MainList().AddChild(new ItemSublist("Anti-Aim", nullptr));
	// Name Stealer
	MainList().AddChild(new ItemSublist("Chat Spam", nullptr));
	MainList().AddChild(new ItemSublist("Misc", nullptr));
	// Info Panel
	MainList().AddChild(new ItemSublist("Ignore Settings", nullptr));
	MainList().AddChild(new ItemSublist("Cheat Settings", nullptr));*/
}

List& MainList() {
	static List* main = List::FromString(R"(
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
    "Auto Sticky" [
        "Auto Sticky Menu"
        "sticky_enabled"
        "sticky_distance"
        "sticky_buildings"
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
        "gui_color_b"
        "gui_color_g"
        "gui_color_r"
        "gui_rainbow"
        "fast_outline"
        "gui_bounds"
        "gui_nullcore"
        "gui_visible"
    ]
    "Miscellaneous" [
        "Miscellaneous"
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
 
"Anti-Aim" [
    "Anti-Aim Menu"
    "aa_enabled"
    "aa_pitch"
    "aa_pitch_mode"
    "aa_yaw"
    "aa_yaw_mode"
    "aa_spin"
    "aa_roll"
    "aa_no_clamp"
    "Anti-Anti-AA" [
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
]

"Chat" [
    "Chat Options Menu"
    "killsay"
    "spam"
    "spam_newlines"
    "clean_chat"
]
 
"Follow Bot" [
    "Follow Bot Settings"
    "fb_bot"
    "fb_mimic_slot"
    "fb_always_medigun"
    "fb_autoclass"
]

"Miscellaneous" [
    "Miscellaneous Settings"
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
    "Airstuck" [
        "Airstuck Menu"
        "airstuck"
        "airstuck_toggle"
    ]
    "bhop_enabled"
    "noisemaker"
    "fast_vischeck"
    "anti_afk"
    "rollspeedhack"
    "Debug" [
        "Debug Menu"
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
