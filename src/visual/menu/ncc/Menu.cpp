/*
 * Menu.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

#include "menu/ncc/Menu.hpp"
#include "menu/ncc/List.hpp"
#include "menu/ncc/ItemSublist.hpp"
#include "menu/ncc/Tooltip.hpp"

namespace menu
{
namespace ncc
{

unsigned long font_title = 0;
unsigned long font_item  = 0;

CatVar scale(CV_FLOAT, XORSTR("gui_ncc_scale"), XORSTR("1"), XORSTR("NCC GUI Scale"),
             XORSTR("Defines scale of NCC gui"), 0.5f, 4.0f);
static CatVar font_family(fonts::family_enum, XORSTR("gui_ncc_font_family"), XORSTR("3"),
                          XORSTR("NCC Font Family"),
                          XORSTR("Defines font family for NCC menu"));
static CatVar font_title_family(fonts::family_enum, XORSTR("gui_ncc_font_title_family"),
                                XORSTR("4"), XORSTR("NCC Title Family"),
                                XORSTR("Defines font family for NCC menu titles"));

Tooltip *tooltip = nullptr;
Root *root       = nullptr;

void ShowTooltip(const std::string &text)
{
    tooltip->Show();
    tooltip->SetText(text);
}

std::vector<CatVar *> FindCatVars(const std::string name)
{
    std::vector<CatVar *> result = {};
    for (auto var : CatVarList())
    {
        if (var->name.find(name) == 0)
            result.push_back(var);
    }
    return result;
}

bool init_done = false;

void ChangeCallback(IConVar *var, const char *pszOldValue, float flOldValue)
{
    if (init_done)
        RefreshFonts();
}

void Init()
{
    root = new Root();
    root->Setup();
    scale.InstallChangeCallback(
        [](IConVar *var, const char *pszOldValue, float flOldValue) {
            if (init_done)
                RefreshFonts();
            logging::Info(XORSTR("Scale Changed"));
            root->HandleCustomEvent(KeyValues::AutoDelete(XORSTR("scale_update")));
        });
    font_family.InstallChangeCallback(ChangeCallback);
    font_title_family.InstallChangeCallback(ChangeCallback);
    init_done = true;
    RefreshFonts();
}

void RefreshFonts()
{
    font_title = g_ISurface->CreateFont();
    font_item  = g_ISurface->CreateFont();
    g_ISurface->SetFontGlyphSet(
        font_title,
        fonts::fonts
            .at(_clamp(0, (int) (fonts::fonts.size() - 1),
                       (int) font_title_family))
            .c_str(),
        psize_font_title * (float) scale, 0, 0, 0, 0x0);
    g_ISurface->SetFontGlyphSet(
        font_item,
        fonts::fonts
            .at(_clamp(0, (int) (fonts::fonts.size() - 1), (int) font_family))
            .c_str(),
        psize_font_item * (float) scale, 0, 0, 0, 0x0);
    root->HandleCustomEvent(KeyValues::AutoDelete(XORSTR("font_update")));
}

// fatal error: template instantiation depth exceeds maximum of 900 (use -ftemplate-depth= to increase the maximum)
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
	"Cathook"
	"Aim Bot" [
	    "Aim Bot Menu"
	    "aimbot_enabled"
	    "aimbot_aimkey"
	    "aimbot_aimkey_mode"
	    "aimbot_autoshoot"
        "aimbot_autoshoot_disguised"
	    "aimbot_hitboxmode"
	    "aimbot_fov"
		"aimbot_fov_draw"
		"aimbot_fov_draw_opacity"
	    "aimbot_prioritymode"
	    "aimbot_charge"
	    "Ignore" [
	        "Ignore Menu"
					"aimbot_ignore_cloak"
					"aimbot_ignore_deadringer"
					"aimbot_ignore_vaccinator"
	        "ignore_taunting"
	        "aimbot_ignore_hoovy"
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
	        "Aim Bot Preferences Menu"
	        "aimbot_silent"
	        "aimbot_hitbox"
	        "aimbot_zoomed"
	        "aimbot_only_when_can_shoot"
	        "aimbot_maxrange"
	        "aimbot_slow"
			"aimbot_auto_unzoom"
			"aimbot_auto_zoom"
			"aimbot_spin_up"
			"aimbot_miss_chance"
			"aimbot_extrapolate"
			"aimbot_target_lock"
			"aimbot_rage_only"
			"aimbot_stickys"
			"aimbot_buildings_other"
			"aimbot_buildings_sentry"
	        "Projectile Aimbot" [
	            "Projectile Aimbot Tweaks"
	            "aimbot_projectile"
	            "aimbot_proj_gravity"
	            "aimbot_proj_speed"
	            "aimbot_huntsman_charge"
	        ]
	    ]
	]
	"Trigger Bot" [
	    "Trigger Bot Menu"
			"trigger_enabled"
			"trigger_key"
			"trigger_key_mode"
			"trigger_hitboxmode"
			"trigger_accuracy"
			"trigger_buildings_sentry"
			"trigger_buildings_other"
			"trigger_stickys"
			"trigger_teammates"
			"trigger_charge"
			"trigger_zoomed"
			"trigger_maxrange"
			"trigger_delay"
			"Auto Vacc" [
				"Auto Vacc"
				"auto_vacc"
				"auto_vacc_default_resist"
				"auto_vacc_blast_pop_ubers"
				"auto_vacc_fire_pop_ubers"
				"auto_vacc_bullet_pop_ubers"
				"auto_vacc_reset_timer"
				"auto_vacc_rocket_range"
				"auto_vacc_blast"
				"auto_vacc_blast_pop_crit"
				"auto_vacc_blast_pop_health"
				"auto_vacc_pyro_range"
				"auto_vacc_afterburn"
				"auto_vacc_fire_pop_pyro"
				"auto_vacc_fire"
				"auto_vacc_sniper_pop"
				"auto_vacc_bullets"
			]
			"Ignore" [
				"Ignore Menu"
					"trigger_ignore_cloak"
					"trigger_ignore_hoovy"
					"trigger_ignore_vaccinator"
			]
			"Auto Backstab" [
			"Auto BackStab Menu"
			"autobackstab"
			"autobackstab_range"
			]
		"Auto Sticky" [
			"Auto Sticky Menu"
			"sticky_enabled"
			"sticky_legit"
			"sticky_buildings"
		]
		"Auto Detonator" [
			"Auto Detonator Menu"
			"detonator_enabled"
			"detonator_legit"
		]
		"Auto Airblast" [
		"Auto Airblast Menu"
		"reflect_enabled"
		"reflect_key"
		"reflect_legit"
		"reflect_dodgeball"
		"reflect_only_idle"
		"reflect_stickybombs"
		"reflect_teammates"
		"reflect_fov"
		"reflect_fov_draw"
		"reflect_fov_draw_opacity"
		]
	]
	"Fake Lag" [
	    "Fake Lag Menu"
			"fakelag"
	]
	"Sequence Exploit" [
			"Sequence Exploit Menu"
			"se_master"
			"se_key"
			"se_doom"
			"se_switch"
			"se_value"
			"se_cart"
			"se_cap"
			"se_cloak"
			"se_stickyspam"
			"se_shoot"
			"se_toggle"
	]
	"Visuals" [
	    "Visuals Menu"
			"skybox_changer"
			"nightmode"
			"thirdperson"
			"thirdperson_angles"
	    	"no_zoom"
			"fov"
			"fov_zoomed"
			"render_zoomed"
			"no_invis"
			"no_hats"
			"antidisguise"
			"no_arms"
			"gui_ncc_scale"
			"gui_ncc_font_family"
			"gui_ncc_font_title_family"
			"fast_outline"
			"show_spectators"
			"ESP" [
			    "ESP Menu"
			    "esp_enabled"
				"ESP Preferences"[
					"ESP Preferences Menu"
					"esp_font_scale"
			    	"esp_box"
					"esp_3d_players"
					"esp_3d_buildings"
					"esp_text_position"
					"esp_expand"
			    	"esp_local"
					"esp_tracers"
					"esp_sightlines"
					"esp_health"
					"esp_bones"
					"esp_vischeck"
					"esp_box_corner_size"
					"esp_entity"
					"esp_entity_id"
					"esp_model_name"
					"esp_weapon_spawners"
					"esp_spellbooks"
					"esp_money"
					"esp_money_red"
					"esp_powerups"
					"esp_show_tank"
					"esp_followbot_id"
				]
				"esp_legit"
				"esp_buildings"
				"esp_teammates"
				"esp_weapon"
				"esp_ubercharge"
				"esp_conds"
				"esp_class"
				"esp_name"
				"esp_distance"
				"Item ESP"[
					"Item ESP Menu"
					"esp_item"
					"esp_item_weapons"
					"esp_item_ammo"
					"esp_item_health"
					"esp_item_powerups"
					"esp_item_adrenaline"
				]
				"Projectile ESP" [
					"Projectile ESP Menu"
					"esp_proj"
					"esp_proj_enemy"
					"esp_proj_stickies"
					"esp_proj_pipes"
					"esp_proj_arrows"
					"esp_proj_rockets"
				]
				"Emoji ESP"[
					"Emoji ESP Menu"
					"esp_emoji"
					"esp_okhand"
					"esp_emoji_size"
				]
				]
			"Chams" [
			"Chams Menu"
			"chams_enable"
			"chams_singlepass"
			"chams_legit"
			"chams_weapons_white"
			"chams_recursive"
			"chams_teammate_buildings"
			"chams_stickies"
			"chams_buildings"
			"chams_ammo"
			"chams_medkits"
			"chams_players"
			"chams_teammates"
			"chams_health"
			"chams_flat"
			"Self Chams"[
			"Self Chams Menu"
			"chams_self"
			"chams_self_team"
			"chams_self_rainbow"
			"chams_self_r"
			"chams_self_g"
			"chams_self_b"
			]
			]
			"Glow" [
			"Glow Menu"
			"glow_enable"
			"glow_solid_when"
			"glow_blur_scale"
			"glow_weapons_white"
			"glow_powerups"
			"glow_teammate_buildings"
			"glow_stickies"
			"glow_buildings"
			"glow_ammo"
			"glow_medkits"
			"glow_players"
			"glow_teammates"
			"glow_health"
			"Self Glow"[
			"Self Glow Menu"
			"glow_self"
			"glow_self_team"
			"glow_self_rainbow"
			"glow_self_r"
			"glow_self_g"
			"glow_self_b"
			]
			]
			"Colors" [
			"Colors Menu"
			"gui_rainbow"
			"gui_color_r"
			"gui_color_g"
			"gui_color_b"
			"gui_color_a"
			"esp_color_blue_r"
			"esp_color_blue_g"
			"esp_color_blue_b"
			"esp_color_red_r"
			"esp_color_red_g"
			"esp_color_red_b"
			]
	]
	"Bunny Hop" [
	    "Bunny Hop Menu"
			"bhop_enabled"
			"auto_strafe"
	]
	"Air Stuck" [
	    "Air Stuck Menu"
			"airstuck"
	]
	"Anti-Aim" [
	"Anti-Aim Menu"
	"aa_enabled"
	"aa_realfakes"
	"aa_pitch_real"
	"aa_yaw_real"
	"aa_pitch_mode_real"
	"aa_yaw_mode_real"
	"aa_pitch"
	"aa_yaw"
	"aa_pitch_mode"
	"aa_yaw_mode"
	"aa_spin"
	"aa_roll"
	"aa_no_clamp"
	"resolver"
	"identify"
	"Anti-/Anti-Aim" [
			"Anti-/Anti-Aim"
			"aa_aaaa_enabled"
			"aa_aaaa_flip_key"
			"aa_aaaa_mode"
			"aa_aaaa_interval_low"
			"aa_aaaa_interval_high"
			"aa_aaaa_interval"
			]
	]
	"Crit Hack" [
			"Crit Hack Menu"
			"crit_key"
			"crit_melee"
			"crit_experimental"
			"crit_force_gameplay"
			"crit_info"
	]
	"Name Stealer" [
	    "Name Stealer Menu"
	    "name_stealer"
	]
	"Chat Settings" [
			"Chat Settings Menu"
			"spam"
			"killsay"
			"killsay_file"
			"spam_teamname"
			"spam_voicecommand"
			"chat_newlines"
			"spam_delay"
			"spam_file"
			"spam_random"
			"clean_chat"
			"chat_censor_enabled"
			"chat_censor"
			"chat_crypto"
			"chat_log"
			"chat_log_nospam"
			"chat_log_noipc"
			"Uberspam" [
				"Uberspam Menu"
					"uberspam"
					"uberspam_file"
					"uberspam_team"
					"uberspam_build"
					"uberspam_ended"
					"uberspam_used"
					"uberspam_ready"
			]
	]
	"Misc" [
	    "Misc Menu"
			"serverlag"
			"deadringer_auto"
			"halloween_mode"
			"name"
			"disconnect_reason"
			"pure_bypass"
			"nolerp"
			"info"
			"info_min"
			"noisemaker"
			"medal_flip"
			"killstreak"
			"tauntslide_tf2"
			"clean_screenshots"
			"anti_afk"
			"die_if_vac"
			"announcer"
			"request_balance_spam"
			"nopush_enabled"
			"bptf_enable"
				"Followbot"[
					"Followbot Menu"
					"fb"
					"fb_autoteam"
					"fb_autoclass"
					"fb_activation"
					"fb_distance"
					"fb_mimic_slot"
					"fb_sync_taunt"
					"fb_always_medigun"
					"fb_roaming"
					"fb_draw"
				]
				"Catbot Utilities"[
					"Catbot Utilities Menu"
					"cbu"
					"cbu_random_votekicks"
					"cbu_micspam"
					"cbu_micspam_on_interval"
					"cbu_micspam_off_interval"
					"cbu_abandon_if_bots_gte"
					"cbu_abandon_if_ipc_bots_gte"
					"cbu_abandon_if_humans_lte"
					"cbu_abandon_if_players_lte"
					"cbu_mark_human_threshold"
				]
				"Anti Backstab" [
				"Anti Backstab Menu"
				"antibackstab"
				"antibackstab_nope"
				"antibackstab_angle"
				"antibackstab_distance"
				]
				"Spyalert" [
				"Spyalert Menu"
				"spyalert_enabled"
				"spyalert_backstab"
				"spyalert_sound"
				"spyalert_interval"
				"spyalert_warning"
				]
				"Backtrack" [
				"Backtrack Menu"
				"backtrack"
				"backtrack_draw"
				"backtrack_latency"
				]
				"Anti Cheat" [
				"Anti Cheat Menu"
				"ac_enabled"
				"ac_aimbot"
				"ac_aimbot_detections"
				"ac_aimbot_angle"
				"ac_antiaim"
				"ac_autorage"
				"ac_bhop_count"
				"ac_ignore_local"
				"ac_chat"
				]
				"Automated" [
				"Automated Menu"
				"anti_votekick"
				"autoqueue"
				"autoqueue_mode"
				"autojoin_team"
				"autojoin_class"
				"autotaunt"
				"autotaunt_chance"
				]
	]
	"Debug" [
		"Debug Menu"
		"fast_vischeck"
		"gui_bounds"
		"debug_freecam"
		"debug_projectiles"
		"debug_log_sent_messages"
		"debug_log_usermessages"
		"debug_tcm"
		"debug_info"
		"votelog"
		"hitrate"
		"skinchanger_debug"
		"debug_aimbot_engine_pp"
		"aimbot_debug"
		"engine_prediction"
		"setupbones_time"
		"debug_ve_averaging"
		"debug_ve_smooth"
		"debug_ve_window"
		"debug_pp_rocket_time_ping"
		"debug_pp_extrapolate"
		"debug_engine_pred_others"
	]
	"God Mode" [
		"God Mode Menu"
		"godmode"
]
)";

List &MainList()
{
    static List *main = List::FromString(IsTF2() ? list_tf2 : list_hl2dm);
    return *main;
}
}
}
