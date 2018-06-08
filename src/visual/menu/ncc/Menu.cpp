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

static const std::string list_hl2dm = RXORSTR("(
XORSTR("Cat Hook")
XORSTR("Aim Bot") [
	XORSTR("Aim Bot Menu")
	XORSTR("aimbot_enabled")
	XORSTR("aimbot_aimkey")
	XORSTR("aimbot_aimkey_mode")
	XORSTR("aimbot_autoshoot")
	XORSTR("aimbot_hitboxmode")
	XORSTR("aimbot_fov")
	XORSTR("aimbot_prioritymode")
	XORSTR("aimbot_charge")
    XORSTR("aimbot_teammates")
	XORSTR("Preferences") [
		XORSTR("Aim Bot Preferences")
		XORSTR("aimbot_silent")
		XORSTR("aimbot_hitbox")
		XORSTR("aimbot_only_when_can_shoot")
		XORSTR("aimbot_enable_attack_only")
		XORSTR("aimbot_maxrange")
		XORSTR("aimbot_interp")
		XORSTR("aimbot_slow")
		XORSTR("aimbot_slow_smooth")
		XORSTR("aimbot_slow_autoshoot")
	]
]
 
XORSTR("Trigger Bot") [
	XORSTR("Trigger Bot Menu")
	XORSTR("trigger_enabled")
	XORSTR("trigger_accuracy")
	XORSTR("trigger_range")
	XORSTR("trigger_hitbox")
]
 


XORSTR("Visuals") [
	XORSTR("Visuals Menu")
	XORSTR("ESP") [
		XORSTR("ESP Menu")
		XORSTR("esp_enabled")
		XORSTR("font_esp_family")
		XORSTR("font_esp_height")
		XORSTR("esp_name")
		XORSTR("esp_distance")
		XORSTR("esp_box")
		XORSTR("esp_box_text_position")
		XORSTR("esp_box_nodraw")
		XORSTR("esp_box_expand")
		XORSTR("3D Box") [
			XORSTR("3D Box Settings")
			XORSTR("esp_3d_box")
			XORSTR("esp_3d_box_thick")
			XORSTR("esp_3d_box_health")
			XORSTR("esp_3d_box_expand")
			XORSTR("esp_3d_box_smoothing")
			XORSTR("esp_3d_box_expand_size")
			XORSTR("esp_3d_box_healthbar")
		]
		XORSTR("esp_legit")
		XORSTR("esp_health_num")
		XORSTR("esp_weapon_spawners")
		XORSTR("esp_model_name")
		XORSTR("esp_weapon")
		XORSTR("esp_vischeck")
		XORSTR("esp_entity_id")
		XORSTR("esp_followbot_id")
		XORSTR("esp_teammates")
		XORSTR("esp_entity")
		XORSTR("esp_local")
		XORSTR("Items") [
			XORSTR("Item ESP Menu")
			XORSTR("esp_item")
			XORSTR("esp_item_adrenaline")
			XORSTR("esp_item_powerups")
			XORSTR("esp_item_health")
			XORSTR("esp_item_ammo")
			XORSTR("esp_item_weapons")
		]
	]
	XORSTR("Chams") [
		XORSTR("Chams Menu")
		XORSTR("chams_enable")
		XORSTR("chams_health")
		XORSTR("chams_players")
		XORSTR("chams_teammates")
		XORSTR("chams_flat")
		XORSTR("chams_weapons")
		XORSTR("chams_medkits")
		XORSTR("chams_ammo")
	]

	XORSTR("Glow") [
		XORSTR("Glow Menu")
		XORSTR("glow_enable")
		XORSTR("glow_solid_when")
		XORSTR("glow_blur_scale")
		XORSTR("glow_health")
		XORSTR("glow_players")
		XORSTR("glow_teammates")
		XORSTR("glow_medkits")
		XORSTR("glow_ammo")
	]
	XORSTR("GUI") [
		XORSTR("GUI Settings")
		XORSTR("logo")
		XORSTR("gui_bg_particles")
		XORSTR("gui_bg_particles_type")
		XORSTR("gui_bg_particles_chance")
		XORSTR("gui_bg_particles_pack_size")
		XORSTR("gui_bg_particles_safe_zone")
		XORSTR("gui_bg_particles_gravity")
		XORSTR("gui_bg_particles_jittering")
		XORSTR("gui_bg_particles_jittering_chance")
		XORSTR("gui_bg_particles_wind")
		XORSTR("gui_bg_visible")
		XORSTR("gui_color_b")
		XORSTR("gui_color_g")
		XORSTR("gui_color_r")
		XORSTR("gui_rainbow")
		XORSTR("fast_outline")
		XORSTR("gui_bounds")
		XORSTR("gui_visible")
	]
	XORSTR("Miscellaneous") [
		XORSTR("Miscellaneous")
		XORSTR("pure_bypass")
		XORSTR("no_arms")
		XORSTR("fov")
		XORSTR("clean_screenshots")
		XORSTR("logo")
	]
]
 
XORSTR("Anti-/Anti-Aim") [
	XORSTR("Anti-Aim Menu")
	XORSTR("aa_enabled")
	XORSTR("aa_pitch")
	XORSTR("aa_pitch_mode")
	XORSTR("aa_yaw")
	XORSTR("aa_yaw_mode")
	XORSTR("aa_spin")
	XORSTR("aa_roll")
	XORSTR("aa_no_clamp")
	XORSTR("resolver")
	XORSTR("Anti-Anti-Anti-Aim") [
		XORSTR("Anti-Anti-Anti-Aim Menu")
		XORSTR("aa_aaaa_enabled")
		XORSTR("aa_aaaa_interval")
		XORSTR("aa_aaaa_interval_low")
		XORSTR("aa_aaaa_interval_high")
		XORSTR("aa_aaaa_mode")
		XORSTR("aa_aaaa_flip_key")
	]
]

XORSTR("Chat") [
	XORSTR("Chat Options Menu")
	XORSTR("chat_newlines")
	XORSTR("clean_chat")
	XORSTR("killsay")
	XORSTR("spam")
	XORSTR("spam_random")
]
 
XORSTR("Follow Bot") [
	XORSTR("Follow Bot Settings")
	XORSTR("fb_bot")
	XORSTR("fb_mimic_slot")
	XORSTR("fb_always_medigun")
	XORSTR("fb_autoclass")
	XORSTR("fb_follow_distance")
]

XORSTR("Miscellaneous") [
	XORSTR("Miscellaneous Settings")
	XORSTR("bhop_enabled")
	XORSTR("fast_vischeck")
	XORSTR("anti_afk")
	XORSTR("rollspeedhack")
	XORSTR("info")
	XORSTR("Debug") [
		XORSTR("Debug Menu")
		XORSTR("software_cursor_mode")
		XORSTR("enabled")
		XORSTR("no_visuals")
		XORSTR("debug_info")
		XORSTR("log")
	]
]
)XORSTR(";

static const std::string list_tf2 = RXORSTR("(
	XORSTR("Cathook")
	XORSTR("Aim Bot") [
	    XORSTR("Aim Bot Menu")
	    XORSTR("aimbot_enabled")
	    XORSTR("aimbot_aimkey")
	    XORSTR("aimbot_aimkey_mode")
	    XORSTR("aimbot_autoshoot")
        XORSTR("aimbot_autoshoot_disguised")
	    XORSTR("aimbot_hitboxmode")
	    XORSTR("aimbot_fov")
		XORSTR("aimbot_fov_draw")
		XORSTR("aimbot_fov_draw_opacity")
	    XORSTR("aimbot_prioritymode")
	    XORSTR("aimbot_charge")
	    XORSTR("Ignore") [
	        XORSTR("Ignore Menu")
					XORSTR("aimbot_ignore_cloak")
					XORSTR("aimbot_ignore_deadringer")
					XORSTR("aimbot_ignore_vaccinator")
	        XORSTR("ignore_taunting")
	        XORSTR("aimbot_ignore_hoovy")
	        XORSTR("aimbot_teammates")
	    ]
	    XORSTR("Auto Heal") [
	        XORSTR("Auto Heal Menu")
	        XORSTR("autoheal_enabled")
	        XORSTR("autoheal_uber")
	        XORSTR("autoheal_uber_health")
	        XORSTR("autoheal_silent")
	        XORSTR("autoheal_share_uber")
	    ]
	    XORSTR("Preferences") [
	        XORSTR("Aim Bot Preferences Menu")
	        XORSTR("aimbot_silent")
	        XORSTR("aimbot_hitbox")
	        XORSTR("aimbot_zoomed")
	        XORSTR("aimbot_only_when_can_shoot")
	        XORSTR("aimbot_maxrange")
	        XORSTR("aimbot_slow")
			XORSTR("aimbot_auto_unzoom")
			XORSTR("aimbot_auto_zoom")
			XORSTR("aimbot_spin_up")
			XORSTR("aimbot_miss_chance")
			XORSTR("aimbot_extrapolate")
			XORSTR("aimbot_target_lock")
			XORSTR("aimbot_rage_only")
			XORSTR("aimbot_stickys")
			XORSTR("aimbot_buildings_other")
			XORSTR("aimbot_buildings_sentry")
	        XORSTR("Projectile Aimbot") [
	            XORSTR("Projectile Aimbot Tweaks")
	            XORSTR("aimbot_projectile")
	            XORSTR("aimbot_proj_gravity")
	            XORSTR("aimbot_proj_speed")
	            XORSTR("aimbot_huntsman_charge")
	        ]
	    ]
	]
	XORSTR("Trigger Bot") [
	    XORSTR("Trigger Bot Menu")
			XORSTR("trigger_enabled")
			XORSTR("trigger_key")
			XORSTR("trigger_key_mode")
			XORSTR("trigger_hitboxmode")
			XORSTR("trigger_accuracy")
			XORSTR("trigger_buildings_sentry")
			XORSTR("trigger_buildings_other")
			XORSTR("trigger_stickys")
			XORSTR("trigger_teammates")
			XORSTR("trigger_charge")
			XORSTR("trigger_zoomed")
			XORSTR("trigger_maxrange")
			XORSTR("trigger_delay")
			XORSTR("Auto Vacc") [
				XORSTR("Auto Vacc")
				XORSTR("auto_vacc")
				XORSTR("auto_vacc_default_resist")
				XORSTR("auto_vacc_blast_pop_ubers")
				XORSTR("auto_vacc_fire_pop_ubers")
				XORSTR("auto_vacc_bullet_pop_ubers")
				XORSTR("auto_vacc_reset_timer")
				XORSTR("auto_vacc_rocket_range")
				XORSTR("auto_vacc_blast")
				XORSTR("auto_vacc_blast_pop_crit")
				XORSTR("auto_vacc_blast_pop_health")
				XORSTR("auto_vacc_pyro_range")
				XORSTR("auto_vacc_afterburn")
				XORSTR("auto_vacc_fire_pop_pyro")
				XORSTR("auto_vacc_fire")
				XORSTR("auto_vacc_sniper_pop")
				XORSTR("auto_vacc_bullets")
			]
			XORSTR("Ignore") [
				XORSTR("Ignore Menu")
					XORSTR("trigger_ignore_cloak")
					XORSTR("trigger_ignore_hoovy")
					XORSTR("trigger_ignore_vaccinator")
			]
			XORSTR("Auto Backstab") [
			XORSTR("Auto BackStab Menu")
			XORSTR("autobackstab")
			XORSTR("autobackstab_range")
			]
		XORSTR("Auto Sticky") [
			XORSTR("Auto Sticky Menu")
			XORSTR("sticky_enabled")
			XORSTR("sticky_legit")
			XORSTR("sticky_buildings")
		]
		XORSTR("Auto Detonator") [
			XORSTR("Auto Detonator Menu")
			XORSTR("detonator_enabled")
			XORSTR("detonator_legit")
		]
		XORSTR("Auto Airblast") [
		XORSTR("Auto Airblast Menu")
		XORSTR("reflect_enabled")
		XORSTR("reflect_key")
		XORSTR("reflect_legit")
		XORSTR("reflect_dodgeball")
		XORSTR("reflect_only_idle")
		XORSTR("reflect_stickybombs")
		XORSTR("reflect_teammates")
		XORSTR("reflect_fov")
		XORSTR("reflect_fov_draw")
		XORSTR("reflect_fov_draw_opacity")
		]
	]
	XORSTR("Fake Lag") [
	    XORSTR("Fake Lag Menu")
			XORSTR("fakelag")
	]
	XORSTR("Sequence Exploit") [
			XORSTR("Sequence Exploit Menu")
			XORSTR("se_master")
			XORSTR("se_key")
			XORSTR("se_doom")
			XORSTR("se_switch")
			XORSTR("se_value")
			XORSTR("se_cart")
			XORSTR("se_cap")
			XORSTR("se_cloak")
			XORSTR("se_stickyspam")
			XORSTR("se_shoot")
			XORSTR("se_toggle")
	]
	XORSTR("Visuals") [
	    XORSTR("Visuals Menu")
			XORSTR("skybox_changer")
			XORSTR("nightmode")
			XORSTR("thirdperson")
			XORSTR("thirdperson_angles")
	    	XORSTR("no_zoom")
			XORSTR("fov")
			XORSTR("fov_zoomed")
			XORSTR("render_zoomed")
			XORSTR("no_invis")
			XORSTR("no_hats")
			XORSTR("antidisguise")
			XORSTR("no_arms")
			XORSTR("gui_ncc_scale")
			XORSTR("gui_ncc_font_family")
			XORSTR("gui_ncc_font_title_family")
			XORSTR("fast_outline")
			XORSTR("show_spectators")
			XORSTR("ESP") [
			    XORSTR("ESP Menu")
			    XORSTR("esp_enabled")
				XORSTR("ESP Preferences")[
					XORSTR("ESP Preferences Menu")
					XORSTR("esp_font_scale")
			    	XORSTR("esp_box")
					XORSTR("esp_3d_players")
					XORSTR("esp_3d_buildings")
					XORSTR("esp_text_position")
					XORSTR("esp_expand")
			    	XORSTR("esp_local")
					XORSTR("esp_tracers")
					XORSTR("esp_sightlines")
					XORSTR("esp_health")
					XORSTR("esp_bones")
					XORSTR("esp_vischeck")
					XORSTR("esp_box_corner_size")
					XORSTR("esp_entity")
					XORSTR("esp_entity_id")
					XORSTR("esp_model_name")
					XORSTR("esp_weapon_spawners")
					XORSTR("esp_spellbooks")
					XORSTR("esp_money")
					XORSTR("esp_money_red")
					XORSTR("esp_powerups")
					XORSTR("esp_show_tank")
					XORSTR("esp_followbot_id")
				]
				XORSTR("esp_legit")
				XORSTR("esp_buildings")
				XORSTR("esp_teammates")
				XORSTR("esp_weapon")
				XORSTR("esp_ubercharge")
				XORSTR("esp_conds")
				XORSTR("esp_class")
				XORSTR("esp_name")
				XORSTR("esp_distance")
				XORSTR("Item ESP")[
					XORSTR("Item ESP Menu")
					XORSTR("esp_item")
					XORSTR("esp_item_weapons")
					XORSTR("esp_item_ammo")
					XORSTR("esp_item_health")
					XORSTR("esp_item_powerups")
					XORSTR("esp_item_adrenaline")
				]
				XORSTR("Projectile ESP") [
					XORSTR("Projectile ESP Menu")
					XORSTR("esp_proj")
					XORSTR("esp_proj_enemy")
					XORSTR("esp_proj_stickies")
					XORSTR("esp_proj_pipes")
					XORSTR("esp_proj_arrows")
					XORSTR("esp_proj_rockets")
				]
				XORSTR("Emoji ESP")[
					XORSTR("Emoji ESP Menu")
					XORSTR("esp_emoji")
					XORSTR("esp_okhand")
					XORSTR("esp_emoji_size")
				]
				]
			XORSTR("Chams") [
			XORSTR("Chams Menu")
			XORSTR("chams_enable")
			XORSTR("chams_singlepass")
			XORSTR("chams_legit")
			XORSTR("chams_weapons_white")
			XORSTR("chams_recursive")
			XORSTR("chams_teammate_buildings")
			XORSTR("chams_stickies")
			XORSTR("chams_buildings")
			XORSTR("chams_ammo")
			XORSTR("chams_medkits")
			XORSTR("chams_players")
			XORSTR("chams_teammates")
			XORSTR("chams_health")
			XORSTR("chams_flat")
			XORSTR("Self Chams")[
			XORSTR("Self Chams Menu")
			XORSTR("chams_self")
			XORSTR("chams_self_team")
			XORSTR("chams_self_rainbow")
			XORSTR("chams_self_r")
			XORSTR("chams_self_g")
			XORSTR("chams_self_b")
			]
			]
			XORSTR("Glow") [
			XORSTR("Glow Menu")
			XORSTR("glow_enable")
			XORSTR("glow_solid_when")
			XORSTR("glow_blur_scale")
			XORSTR("glow_weapons_white")
			XORSTR("glow_powerups")
			XORSTR("glow_teammate_buildings")
			XORSTR("glow_stickies")
			XORSTR("glow_buildings")
			XORSTR("glow_ammo")
			XORSTR("glow_medkits")
			XORSTR("glow_players")
			XORSTR("glow_teammates")
			XORSTR("glow_health")
			XORSTR("Self Glow")[
			XORSTR("Self Glow Menu")
			XORSTR("glow_self")
			XORSTR("glow_self_team")
			XORSTR("glow_self_rainbow")
			XORSTR("glow_self_r")
			XORSTR("glow_self_g")
			XORSTR("glow_self_b")
			]
			]
			XORSTR("Colors") [
			XORSTR("Colors Menu")
			XORSTR("gui_rainbow")
			XORSTR("gui_color_r")
			XORSTR("gui_color_g")
			XORSTR("gui_color_b")
			XORSTR("gui_color_a")
			XORSTR("esp_color_blue_r")
			XORSTR("esp_color_blue_g")
			XORSTR("esp_color_blue_b")
			XORSTR("esp_color_red_r")
			XORSTR("esp_color_red_g")
			XORSTR("esp_color_red_b")
			]
	]
	XORSTR("Bunny Hop") [
	    XORSTR("Bunny Hop Menu")
			XORSTR("bhop_enabled")
			XORSTR("auto_strafe")
	]
	XORSTR("Air Stuck") [
	    XORSTR("Air Stuck Menu")
			XORSTR("airstuck")
	]
	XORSTR("Anti-Aim") [
	XORSTR("Anti-Aim Menu")
	XORSTR("aa_enabled")
	XORSTR("aa_realfakes")
	XORSTR("aa_pitch_real")
	XORSTR("aa_yaw_real")
	XORSTR("aa_pitch_mode_real")
	XORSTR("aa_yaw_mode_real")
	XORSTR("aa_pitch")
	XORSTR("aa_yaw")
	XORSTR("aa_pitch_mode")
	XORSTR("aa_yaw_mode")
	XORSTR("aa_spin")
	XORSTR("aa_roll")
	XORSTR("aa_no_clamp")
	XORSTR("resolver")
	XORSTR("identify")
	XORSTR("Anti-/Anti-Aim") [
			XORSTR("Anti-/Anti-Aim")
			XORSTR("aa_aaaa_enabled")
			XORSTR("aa_aaaa_flip_key")
			XORSTR("aa_aaaa_mode")
			XORSTR("aa_aaaa_interval_low")
			XORSTR("aa_aaaa_interval_high")
			XORSTR("aa_aaaa_interval")
			]
	]
	XORSTR("Crit Hack") [
			XORSTR("Crit Hack Menu")
			XORSTR("crit_key")
			XORSTR("crit_melee")
			XORSTR("crit_experimental")
			XORSTR("crit_force_gameplay")
			XORSTR("crit_info")
	]
	XORSTR("Name Stealer") [
	    XORSTR("Name Stealer Menu")
	    XORSTR("name_stealer")
	]
	XORSTR("Chat Settings") [
			XORSTR("Chat Settings Menu")
			XORSTR("spam")
			XORSTR("killsay")
			XORSTR("killsay_file")
			XORSTR("spam_teamname")
			XORSTR("spam_voicecommand")
			XORSTR("chat_newlines")
			XORSTR("spam_delay")
			XORSTR("spam_file")
			XORSTR("spam_random")
			XORSTR("clean_chat")
			XORSTR("chat_censor_enabled")
			XORSTR("chat_censor")
			XORSTR("chat_crypto")
			XORSTR("chat_log")
			XORSTR("chat_log_nospam")
			XORSTR("chat_log_noipc")
			XORSTR("Uberspam") [
				XORSTR("Uberspam Menu")
					XORSTR("uberspam")
					XORSTR("uberspam_file")
					XORSTR("uberspam_team")
					XORSTR("uberspam_build")
					XORSTR("uberspam_ended")
					XORSTR("uberspam_used")
					XORSTR("uberspam_ready")
			]
	]
	XORSTR("Misc") [
	    XORSTR("Misc Menu")
			XORSTR("serverlag")
			XORSTR("deadringer_auto")
			XORSTR("halloween_mode")
			XORSTR("name")
			XORSTR("disconnect_reason")
			XORSTR("pure_bypass")
			XORSTR("nolerp")
			XORSTR("info")
			XORSTR("info_min")
			XORSTR("noisemaker")
			XORSTR("medal_flip")
			XORSTR("killstreak")
			XORSTR("tauntslide_tf2")
			XORSTR("clean_screenshots")
			XORSTR("anti_afk")
			XORSTR("die_if_vac")
			XORSTR("announcer")
			XORSTR("request_balance_spam")
			XORSTR("nopush_enabled")
			XORSTR("bptf_enable")
				XORSTR("Followbot")[
					XORSTR("Followbot Menu")
					XORSTR("fb")
					XORSTR("fb_autoteam")
					XORSTR("fb_autoclass")
					XORSTR("fb_activation")
					XORSTR("fb_distance")
					XORSTR("fb_mimic_slot")
					XORSTR("fb_sync_taunt")
					XORSTR("fb_always_medigun")
					XORSTR("fb_roaming")
					XORSTR("fb_draw")
				]
				XORSTR("Catbot Utilities")[
					XORSTR("Catbot Utilities Menu")
					XORSTR("cbu")
					XORSTR("cbu_random_votekicks")
					XORSTR("cbu_micspam")
					XORSTR("cbu_micspam_on_interval")
					XORSTR("cbu_micspam_off_interval")
					XORSTR("cbu_abandon_if_bots_gte")
					XORSTR("cbu_abandon_if_ipc_bots_gte")
					XORSTR("cbu_abandon_if_humans_lte")
					XORSTR("cbu_abandon_if_players_lte")
					XORSTR("cbu_mark_human_threshold")
				]
				XORSTR("Anti Backstab") [
				XORSTR("Anti Backstab Menu")
				XORSTR("antibackstab")
				XORSTR("antibackstab_nope")
				XORSTR("antibackstab_angle")
				XORSTR("antibackstab_distance")
				]
				XORSTR("Spyalert") [
				XORSTR("Spyalert Menu")
				XORSTR("spyalert_enabled")
				XORSTR("spyalert_backstab")
				XORSTR("spyalert_sound")
				XORSTR("spyalert_interval")
				XORSTR("spyalert_warning")
				]
				XORSTR("Backtrack") [
				XORSTR("Backtrack Menu")
				XORSTR("backtrack")
				XORSTR("backtrack_draw")
				XORSTR("backtrack_latency")
				]
				XORSTR("Anti Cheat") [
				XORSTR("Anti Cheat Menu")
				XORSTR("ac_enabled")
				XORSTR("ac_aimbot")
				XORSTR("ac_aimbot_detections")
				XORSTR("ac_aimbot_angle")
				XORSTR("ac_antiaim")
				XORSTR("ac_autorage")
				XORSTR("ac_bhop_count")
				XORSTR("ac_ignore_local")
				XORSTR("ac_chat")
				]
				XORSTR("Automated") [
				XORSTR("Automated Menu")
				XORSTR("anti_votekick")
				XORSTR("autoqueue")
				XORSTR("autoqueue_mode")
				XORSTR("autojoin_team")
				XORSTR("autojoin_class")
				XORSTR("autotaunt")
				XORSTR("autotaunt_chance")
				]
	]
	XORSTR("Debug") [
		XORSTR("Debug Menu")
		XORSTR("fast_vischeck")
		XORSTR("gui_bounds")
		XORSTR("debug_freecam")
		XORSTR("debug_projectiles")
		XORSTR("debug_log_sent_messages")
		XORSTR("debug_log_usermessages")
		XORSTR("debug_tcm")
		XORSTR("debug_info")
		XORSTR("votelog")
		XORSTR("hitrate")
		XORSTR("skinchanger_debug")
		XORSTR("debug_aimbot_engine_pp")
		XORSTR("aimbot_debug")
		XORSTR("engine_prediction")
		XORSTR("setupbones_time")
		XORSTR("debug_ve_averaging")
		XORSTR("debug_ve_smooth")
		XORSTR("debug_ve_window")
		XORSTR("debug_pp_rocket_time_ping")
		XORSTR("debug_pp_extrapolate")
		XORSTR("debug_engine_pred_others")
	]
	XORSTR("God Mode") [
		XORSTR("God Mode Menu")
		XORSTR("godmode")
]
)XORSTR(";

List &MainList()
{
    static List *main = List::FromString(IsTF2() ? list_tf2 : list_hl2dm);
    return *main;
}
}
}
