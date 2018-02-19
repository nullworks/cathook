#include "common.hpp"
#include "imgui/imgui.h"
#include "gui/menu.hpp"
int tabs = 1;
//IMPORTANT
void enableGUI()
{   
    if (GUI) // needed, otherwise tf2 crashes upon attaching
    {
        gui_visible = true;
    }
    else
    {
        gui_visible = false;
    }
}
//IMPORTANT

//aimbot
bool aimbot = false;
bool aimbot_autoshoot = true;
bool aimbot_auto_unzoom = false;
bool aimbot_auto_zoom = false;
bool aimbot_buildings_other = true;
bool aimbot_buildings_sentry = true;
bool aimbot_charge = false;
bool aimbot_fov_draw = false;
bool aimbot_ignore_cloak = true;
bool aimbot_ignore_deadringer = true;
bool aimbot_ignore_hoovy = false;
bool aimbot_ignore_vaccinator = true;
bool aimbot_only_when_can_shoot = true;
bool aimbot_projectile = true;
bool aimbot_silent = false;
bool aimbot_stickys = false;
bool aimbot_target_lock = false;
bool aimbot_zoomed = false;
bool autoheal_enabled = false;
bool autoheal_share_uber = true;
bool autoheal_silent = true;
bool autoheal_uber = true;
bool ignore_taunting_sec = true;
int aimbot_fov = 0;
int aimbot_fov_check = 0;
float aimbot_fov_draw_opacity = 0.7f;
float aimbot_fov_draw_opacity_check = 0.7f;
float aimbot_huntsman_charge = 0.5;
float aimbot_huntsman_charge_check = 0.5;
int aimbot_maxrange = 0;
int aimbot_maxrange_check = 0;
float aimbot_miss_chance = 0;
float aimbot_miss_chance_check = 0;
int aimbot_proj_speed = 0;
int aimbot_proj_speed_check = 0;
float aimbot_slow = 0;
float aimbot_slow_check = 0;
int autoheal_uber_health = 30;
int autoheal_uber_health_check = 30;
int aimbot_hitbox = 0;
int aimbot_hitbox_check = 0;
int aimbot_hitboxmode = 0;
int aimbot_hitboxmode_check = 0;
int aimbot_prioritymode = 0;
int aimbot_prioritymode_check = 0;
int aimbot_teammates = 0;
int aimbot_teammates_check = 0;

int enable_var = 0;
int enable_var1 = 0;
int enable_var2 = 0;
int enable_var3 = 0;
int enable_var4 = 0;
int enable_var5 = 0;
int enable_var6 = 0;
int enable_var7 = 0;
int enable_var8 = 0;
int enable_var9 = 0;
int enable_var10 = 0;
int enable_var11 = 0;
int enable_var12 = 0;
int enable_var13 = 0;
int enable_var14 = 0;
int enable_var15 = 0;
int enable_var16 = 0;
int enable_var17 = 0;
int enable_var18 = 0;
int enable_var19 = 0;
int enable_var20 = 0;
int enable_var21 = 0;
int enable_var22 = 0;

//triggerbot
bool autobackstab = false;
bool auto_vacc = false;
bool auto_vacc_afterburn = true;
bool auto_vacc_blast = true;
bool auto_vacc_blast_pop_crit = true;
bool auto_vacc_bullets = true;
bool auto_vacc_fire = true;
bool reflect_dodgeball = false;
bool reflect_enabled = false;
bool reflect_legit = false;
bool reflect_only_idle = false;
bool reflect_stickybombs = false;
bool sticky_buildings = true;
bool sticky_enabled = false;
bool sticky_legit = false;
bool trigger_accuracy = true;
bool trigger_buildings_other = true;
bool trigger_buildings_sentry = true;
bool trigger_charge = false;
bool trigger_enabled = false;
bool trigger_ignore_cloak = true;
bool trigger_ignore_hoovy = true;
bool trigger_ignore_vaccinator = true;
bool trigger_stickys = true;
bool trigger_teammates = false;
bool trigger_zoomed = true;
float trigger_delay = 0;
float trigger_delay_check = 0;
int auto_vacc_blast_pop_health = 80;
int auto_vacc_blast_pop_health_check = 80;
int auto_vacc_blast_pop_ubers = 0;
int auto_vacc_bullet_pop_ubers = 0;
int auto_vacc_fire_pop_ubers = 0;
int auto_vacc_blast_pop_ubers_check = 0;
int auto_vacc_bullet_pop_ubers_check = 0;
int auto_vacc_fire_pop_ubers_check = 0;
int auto_vacc_pyro_range = 450;
int auto_vacc_pyro_range_check = 450;
int auto_vacc_reset_timer = 200;
int auto_vacc_rocket_range = 650;
int auto_vacc_reset_timer_check = 200;
int auto_vacc_rocket_range_check = 650;
int trigger_maxrange = 0;
int trigger_maxrange_check = 0;
int auto_vacc_default_resist = 0;
int auto_vacc_default_resist_check = 0;
int auto_vacc_fire_pop_pyro = 1;
int auto_vacc_fire_pop_pyro_check = 0;
int auto_vacc_sniper_pop = 1;
int auto_vacc_sniper_pop_check = 0;
int trigger_hitboxmode = 0;
int trigger_hitboxmode_check = 0;
int trigger_key_mode = 1;
int trigger_key_mode_check = 0;

int enable_2var = 0;
int enable_2var1 = 0;
int enable_2var2 = 0;
int enable_2var3 = 0;
int enable_2var4 = 0;
int enable_2var5 = 0;
int enable_2var6 = 0;
int enable_2var7 = 0;
int enable_2var8 = 0;
int enable_2var9 = 0;
int enable_2var10 = 0;
int enable_2var11 = 0;
int enable_2var12 = 0;
int enable_2var13 = 0;
int enable_2var14 = 0;
int enable_2var15 = 0;
int enable_2var16 = 0;
int enable_2var17 = 0;
int enable_2var18 = 0;
int enable_2var19 = 0;
int enable_2var20 = 0;
int enable_2var21 = 0;
int enable_2var22 = 0;
int enable_2var23 = 0;
int enable_2var24 = 0;
int enable_2var25 = 0;

// Visual/ESP
bool esp_enabled = false;
int esp_box = 2;
int esp_box_check = 2;
int esp_expand = 0;
int esp_expand_check = 0;
int esp_box_corner_size = 10;
int esp_box_corner_size_check = 10;
int esp_health = 3;
int esp_health_check = 3;
int esp_tracers = 0;
int esp_tracers_check = 0;
int esp_text_position = 0;
int esp_text_position_check = 0;
bool esp_legit = false;
bool esp_vischeck = true;
bool esp_bones = false;
bool esp_buildings = true;
bool esp_teammates = false;
bool esp_name = true;
bool esp_class = true;
bool esp_distance = true;
bool esp_conds = true;
bool esp_ubercharge = true;
bool esp_weapon = false;
bool esp_powerups = true;
bool esp_show_tank = true;
bool esp_weapon_spawners = true;
bool esp_followbot_id = true;
bool esp_local = true;
int esp_sightlines = 0;
int esp_sightlines_check = 0;
bool esp_entity = false;
bool esp_model_name = false;
bool esp_entity_id = true;
bool esp_spellbooks = true;

int enable_3var = 0;
int enable_3var1 = 0;
int enable_3var2 = 0;
int enable_3var3 = 0;
int enable_3var4 = 0;
int enable_3var5 = 0;
int enable_3var6 = 0;
int enable_3var7 = 0;
int enable_3var8 = 0;
int enable_3var9 = 0;
int enable_3var10 = 0;
int enable_3var11 = 0;
int enable_3var12 = 0;
int enable_3var13 = 0;
int enable_3var14 = 0;
int enable_3var15 = 0;
int enable_3var16 = 0;
int enable_3var17 = 0;
int enable_3var18 = 0;
int enable_3var19 = 0;
int enable_3var20 = 0;

int esp_color_red_r = 0;
int esp_color_red_g = 0;
int esp_color_red_b = 0;
int esp_color_blue_r = 0;
int esp_color_blue_g = 0;
int esp_color_blue_b = 0;

bool esp_item = true;
bool esp_item_adrenaline = false;
bool esp_item_powerups = true;
bool esp_item_health = true;
bool esp_item_ammo = false; 
bool esp_item_weapons = false;
bool esp_money_red = true;
bool esp_money = true;

int enable_4var = 0;
int enable_4var1 = 0;
int enable_4var2 = 0;
int enable_4var3 = 0;
int enable_4var4 = 0;
int enable_4var5 = 0;
int enable_4var6 = 0;
int enable_4var7 = 0;
int enable_4var8 = 0;

bool esp_proj = true;
bool esp_proj_enemy = true;
int esp_proj_stickies = 1;
int esp_proj_stickies_check = 1;
int esp_proj_pipes = 1;
int esp_proj_pipes_check = 1;
int esp_proj_arrows = 1;
int esp_proj_arrows_check = 1;
int esp_proj_rockets = 1;
int esp_proj_rockets_check = 1;

int enable_5var = 0;
int enable_5var1 = 0;

bool chams_enable = false;
bool chams_legit = false;
bool chams_singlepass = false;
bool chams_health = false;
bool chams_players = true;
bool chams_teammates = false;
bool chams_buildings = false;
bool chams_teammate_buildings = false;
bool chams_flat = false;
bool chams_recursive = true;
bool chams_weapons_white = true;
bool chams_medkits = false;
bool chams_ammo = false;
bool chams_stickies = false;

int enable_6var = 0;
int enable_6var1 = 0;
int enable_6var2 = 0;
int enable_6var3 = 0;
int enable_6var4 = 0;
int enable_6var5 = 0;
int enable_6var6 = 0;
int enable_6var7 = 0;
int enable_6var8 = 0;
int enable_6var9 = 0;
int enable_6var10 = 0;
int enable_6var11 = 0;
int enable_6var12 = 0;
int enable_6var13 = 0;

void menu()
{

    aimbot_fov_check = aimbot_fov;
    aimbot_fov_draw_opacity_check = aimbot_fov_draw_opacity;
    aimbot_prioritymode_check = aimbot_prioritymode;
    aimbot_hitboxmode_check = aimbot_hitboxmode;
    aimbot_teammates_check = aimbot_teammates;
    autoheal_uber_health_check = autoheal_uber_health;
    aimbot_miss_chance_check = aimbot_miss_chance;
    aimbot_maxrange_check = aimbot_maxrange;
    aimbot_hitbox_check = aimbot_hitbox;
    aimbot_slow_check = aimbot_slow;
    aimbot_proj_speed_check = aimbot_proj_speed;
    aimbot_huntsman_charge_check = aimbot_huntsman_charge;
    auto_vacc_default_resist_check = auto_vacc_default_resist;
    auto_vacc_reset_timer_check = auto_vacc_reset_timer;
    auto_vacc_rocket_range_check = auto_vacc_rocket_range;
    auto_vacc_blast_pop_health_check = auto_vacc_blast_pop_health;
    auto_vacc_pyro_range_check = auto_vacc_pyro_range;
    auto_vacc_sniper_pop_check = auto_vacc_sniper_pop;
    auto_vacc_fire_pop_pyro_check = auto_vacc_fire_pop_pyro;
    auto_vacc_bullet_pop_ubers_check = auto_vacc_bullet_pop_ubers;
    auto_vacc_fire_pop_ubers_check = auto_vacc_fire_pop_ubers;
    auto_vacc_blast_pop_ubers_check = auto_vacc_blast_pop_ubers;
    trigger_hitboxmode_check = trigger_hitboxmode;
    trigger_delay_check = trigger_delay;
    trigger_maxrange_check = trigger_maxrange;
    trigger_key_mode_check = trigger_key_mode;

    esp_expand_check = esp_expand;
    esp_box_check = esp_box;
    esp_box_corner_size_check = esp_box_corner_size;
    esp_health_check = esp_health;
    esp_tracers_check = esp_tracers;
    esp_text_position_check = esp_text_position;
    esp_sightlines_check = esp_sightlines;

    esp_proj_stickies_check = esp_proj_stickies;
    esp_proj_pipes_check = esp_proj_pipes;
    esp_proj_arrows_check = esp_proj_arrows;
    esp_proj_rockets_check = esp_proj_rockets;

	ImGui::Begin("Config Menu", NULL, ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar);
    if (ImGui::Button("Aimbot##tab"))
    {
        tabs = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Triggerbot"))
    {
        tabs = 2;
    }
    ImGui::SameLine();
    if (ImGui::Button("Visuals"))
    {
        tabs = 3;
    }
    ImGui::SameLine();
    if (ImGui::Button("Anti-/Anti-Aim"))
    {
        tabs = 4;
    }    
    ImGui::SameLine();
    if (ImGui::Button("Crit Hack"))
    {
        tabs = 5;
    }    
    ImGui::SameLine();
    if (ImGui::Button("Chat Settings"))
    {
        tabs = 6;
    }    
    ImGui::SameLine();
    if (ImGui::Button("Anti-Cheat"))
    {
        tabs = 7;
    }    
    ImGui::SameLine();
    if (ImGui::Button("Miscellaneous"))
    {
        tabs = 8;
    } 
    switch (tabs)
    {
        case 1:
        {
            ImGui::BeginChild(1, ImVec2(0,120), false);
            ImGui::Columns(3, "##maincolumn", false);
            ImGui::Checkbox("Enable Aimbot", &aimbot);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Main aimbot switch");
            ImGui::Checkbox("Enable Autoshoot", &aimbot_autoshoot);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Shoot automatically when the target is locked, isn't compatible with 'Enable when attacking'");
            ImGui::Checkbox("Wait for sniper rifle charge", &aimbot_charge);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Aimbot waits until it has enough charge to kill");
            ImGui::Text("Aimbot FOV");
            ImGui::SliderInt("##aimbotfov", &aimbot_fov, 0, 60);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("FOV range for aimbot to lock targets.");
            ImGui::NextColumn();
            ImGui::Checkbox("Draw Fov Ring", &aimbot_fov_draw);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Draws a ring to represent your current aimbot fov");
            ImGui::Text("FOV Circle Opacity");
            ImGui::SliderFloat("##fovdrawopacity", &aimbot_fov_draw_opacity, 0.0f, 1.0f);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Defines opacity of FOV circle");
            ImGui::Text("Priority Mode");
            ImGui::Combo("##prioritymode", &aimbot_prioritymode, "SMART\0FOV\0DISTANCE\0HEALTH\0");
            ImGui::NextColumn();
            ImGui::Text("Hitbox Mode");
            ImGui::Combo("##hitboxmode", &aimbot_hitboxmode, "AUTO\0AUTO-CLOSEST\0STATIC\0");
            ImGui::Checkbox("Auto zoom", &aimbot_auto_zoom);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Automatically zoom");
            ImGui::Checkbox("Auto Un-zoom", &aimbot_auto_unzoom);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Automatically unzoom");
            ImGui::EndChild();
            if (ImGui::TreeNode("Ignore"))
            {
                ImGui::BeginChild(2, ImVec2(0,90), false);
                ImGui::Columns(3, "##ignorecolumn", false);
                ImGui::Checkbox("Ignore cloaked", &aimbot_ignore_cloak);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Don't aim at invisible enemies");
                ImGui::Checkbox("Ignore taunting", &ignore_taunting_sec);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Aimbot/Triggerbot won't attack taunting enemies");
                ImGui::Checkbox("Aim Sentry", &aimbot_buildings_sentry);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Should aimbot aim at sentryguns");
                ImGui::NextColumn();
                ImGui::Checkbox("Ignore deadringer", &aimbot_ignore_deadringer);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Don't aim at deadringed enemies");
                ImGui::Checkbox("Ignore Hoovies", &aimbot_ignore_hoovy);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Aimbot won't attack hoovies");
                ImGui::Checkbox("Aim Other buildings", &aimbot_buildings_other);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Should aimbot aim at other buildings");
                ImGui::NextColumn();
                ImGui::Checkbox("Ignore Vaccinator", &aimbot_ignore_vaccinator);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Hitscan weapons won't fire if enemy is vaccinated against bullets");
                ImGui::Text("Aim Target");
                ImGui::Combo("##aimtarget", &aimbot_teammates, "ENEMY ONLY\0TEAMMATE ONLY\0BOTH\0");
                ImGui::Checkbox("Aim Sticky", &aimbot_stickys);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Should aimbot aim at stickys");
                ImGui::EndChild();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Auto Heal"))
            {   
                ImGui::BeginChild(3, ImVec2(0,50), false);
                ImGui::Columns(3, "##autohealcolumn", false);
                ImGui::Checkbox("AutoHeal", &autoheal_enabled);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Automatically heals nearby teammates");
                ImGui::Checkbox("AutoUber", &autoheal_uber);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Use ubercharge automatically");
                ImGui::NextColumn();
                ImGui::Checkbox("Silent AutoHeal", &autoheal_silent);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Silent AutoHeal. Disable this to make ghetto followbot");
                ImGui::Checkbox("Share ubercharge", &autoheal_share_uber);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Aimbot will attempt to share uber charge with un-ubered players");
                ImGui::NextColumn();
                ImGui::Text("Pop uber if health under");
                ImGui::SliderInt("##uberhealth", &autoheal_uber_health, 0, 100);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("When under a percentage of health, use ubercharge");
                ImGui::EndChild();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Preferences"))
            {
                ImGui::BeginChild(4, ImVec2(0, 100), false);
                ImGui::Columns(3, "##preferences column", false);
                ImGui::Checkbox("Silent", &aimbot_silent);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Your screen doesn't get snapped to the point where aimbot aims at");
                ImGui::Checkbox("Target Lock", &aimbot_target_lock);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Keeps your previously chosen target untill target check fails");
                ImGui::Checkbox("Active when can shoot", &aimbot_only_when_can_shoot);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Aimbot only activates when you can instantly\n shoot, sometimes making the autoshoot invisible\n for spectators");
                ImGui::Checkbox("Zoomed only", &aimbot_zoomed);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Don't autoshoot with unzoomed rifles");
                ImGui::NextColumn();
                ImGui::Text("Miss chance");
                ImGui::SliderFloat("##misschance", &aimbot_miss_chance, 0.0f, 1.0f);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("From 0 to 1. Aimbot will NOT aim in these % cases");
                ImGui::Text("Max distance");
                ImGui::SliderInt("##maxrange", &aimbot_maxrange, 0, 4096);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Max range for aimbot\n 900-1100 range is efficient for scout/widowmaker engineer");
                ImGui::NextColumn();
                ImGui::Text("Hitbox");
                ImGui::Combo("##Hitboxuh", &aimbot_hitbox, "HEAD\0PELVIS\0SPINE 0\0SPINE 1\0SPINE 2\0SPINE 3\0UPPER ARM L\0LOWER ARM L\0HAND L\0UPPER ARM R\0LOWER ARM R\0HAND R\0HIP L\0KNEE L\0FOOT L\0HIP R\0KNEE R\0FOOT R\0");
                ImGui::Text("Slow Aim");
                ImGui::DragFloat("##slowaim", &aimbot_slow, 0.1, 0, 10);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Slowly moves your crosshair onto the target for more legit play\nDisables silent aimbot");
                ImGui::EndChild();
                if (ImGui::TreeNode("Projectile Aimbot Tweaks"))
                {
                    ImGui::Columns(3, "##projectileaimbottweaks", false);
                    ImGui::Checkbox("Projectile aimbot", &aimbot_projectile);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("If you turn it off, aimbot won't try to aim with projectile weapons");
                    ImGui::NextColumn();
                    ImGui::Text("Projectile speed");
                    ImGui::DragInt("##projectilespeed", &aimbot_proj_speed, 1, 0, 20000);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Force override projectile speed.\nCan be useful for playing with MvM upgrades or on x10 serverssince there is no \"automatic\" projectile speed detection in "
               "cathook. Yet.");
                    ImGui::NextColumn();
                    ImGui::Text("Huntsman autoshoot");
                    ImGui::SliderFloat("##huntsmancharge", &aimbot_huntsman_charge, 0.0f, 1.0f);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Minimum charge for autoshooting with huntsman.\n Set it to 0.01 if you want to shoot as soon as you start pulling the arrow");
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
        break;
        
        case 2:
        {
            ImGui::BeginChild(5, ImVec2(0,25), false);
            ImGui::Columns(2, "##triggerbutt", false);
            ImGui::Checkbox("Enable Triggerbot", &trigger_enabled);
            ImGui::NextColumn();
            ImGui::Checkbox("Auto Autobackstab", &autobackstab);
            ImGui::EndChild();
            if (ImGui::TreeNode("Auto Vaccinator"))
            {   
                ImGui::BeginChild(6, ImVec2(0, 200), false);
                ImGui::Columns(3, "##vacc", false);
                ImGui::Checkbox("Auto Vaccinator##checkbox", &auto_vacc);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Pick resistance for incoming damage types");
                ImGui::Checkbox("Check bullet danger", &auto_vacc_bullets);
                ImGui::Checkbox("Pop Blast if crit rocket near", &auto_vacc_blast_pop_crit);
                ImGui::Checkbox("Anti-Afterburn", &auto_vacc_afterburn);
                ImGui::Checkbox("Check blast danger", &auto_vacc_blast);
                ImGui::Checkbox("Check fire danger", &auto_vacc_fire);
                ImGui::Text("Default Resistance");
                ImGui::Combo("##Defaultrest", &auto_vacc_default_resist, "BULLET\0BLAST\0FIRE\0");
                ImGui::NextColumn();
                ImGui::Text("Reset Timer");
                ImGui::SliderInt("##vaccreset", &auto_vacc_reset_timer, 0, 400);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("If no dangers were detected for %d ticks", auto_vacc_reset_timer);
                ImGui::Text("Rocket Danger Range");
                ImGui::SliderInt("##vaccrocketrange", &auto_vacc_rocket_range, 0, 4000);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("This range should be high enough to give more time to change resistances");
                ImGui::Text("Pop Blast if rocket & HP <");
                ImGui::SliderInt("##vaccblastpophealth", &auto_vacc_blast_pop_health, 0, 400);
                ImGui::Text("Pyro Danger Range");
                ImGui::SliderInt("##vaccpyrorange", &auto_vacc_pyro_range, 0, 4000);
                ImGui::Text("Pop if Sniper");
                ImGui::Combo("##popifsniper", &auto_vacc_sniper_pop, "NEVER\0ZOOM & VISIBLE\0ANY ZOOMED\0");
                ImGui::NextColumn();
                ImGui::Text("Pop if pyro is near");
                ImGui::Combo("##poppyronear", &auto_vacc_fire_pop_pyro, "NEVER\0PRIMARY OUT\0ALWAYS\0");
                ImGui::Text("Pop Bullet if Ubers >=");
                ImGui::SliderInt("##vaccbulletpopubers", &auto_vacc_bullet_pop_ubers, 0, 4);
                ImGui::Text("Pop Fire if Ubers >=");
                ImGui::SliderInt("##vaccfirepopubers", &auto_vacc_fire_pop_ubers, 0, 4);
                ImGui::Text("Pop Blast if Ubers >=");
                ImGui::SliderInt("##vaccblastpopubers", &auto_vacc_blast_pop_ubers, 0, 4);
                ImGui::EndChild();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Auto sticky"))
            {   
                ImGui::BeginChild(7, ImVec2(0, 25), false);
                ImGui::Columns(3, "##autostickyc", false);
                ImGui::Checkbox("AutoSticky", &sticky_enabled);
                ImGui::NextColumn();
                ImGui::Checkbox("Detonate buildings", &sticky_buildings);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Stickies react to buildings");
                ImGui::NextColumn();
                ImGui::Checkbox("Legit", &sticky_legit);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Stickys only detonate when you see them\nAlso ignores invis spies");
                ImGui::EndChild();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Auto Reflect"))
            {
                ImGui::BeginChild(8, ImVec2(0, 50), false);
                ImGui::Columns(3, "##autoreflect", false);
                ImGui::Checkbox("Master AutoReflect switch", &reflect_enabled);
                ImGui::Text("Bind cat_reflect_key to a key");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Hold this key to activate auto-airblast");
                ImGui::NextColumn();
                ImGui::Checkbox("Legit Reflect", &reflect_legit);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Only Auto-airblasts projectiles that you can see, doesnt move your crosshair");
                ImGui::Checkbox("Only when not shooting", &reflect_only_idle);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Don't AutoReflect if you're holding M1");
                ImGui::NextColumn();
                ImGui::Checkbox("Reflect stickies", &reflect_stickybombs);
                ImGui::Checkbox("Dodgeball Mode", &reflect_dodgeball);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Allows auto-reflect to work in dodgeball servers");
                ImGui::EndChild();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Triggerbot Ignores"))
            {   
                ImGui::BeginChild(9, ImVec2(0,75), false);
                ImGui::Columns(3, "##Triggerignores", false);
                ImGui::Checkbox("Ignore cloaked", &trigger_ignore_cloak);
                ImGui::Checkbox("Ignore Hoovies", &trigger_ignore_hoovy);
                ImGui::Checkbox("Ignore Vaccinator", &trigger_ignore_vaccinator);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Hitscan weapons won't fire if enemy is vaccinated against bullets");
                ImGui::NextColumn();
                ImGui::Checkbox("Trigger Sentry", &trigger_buildings_sentry);
                ImGui::Checkbox("Trigger Other building", &trigger_buildings_other);
                ImGui::NextColumn();
                ImGui::Checkbox("Trigger Sticky", &trigger_stickys);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Allows auto-reflect to work in dodgeball servers");
                ImGui::Checkbox("Trigger teammates", &trigger_teammates);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Trigger at your own team. Useful for HL2DM");
                ImGui::EndChild();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Triggerbot Preferences"))
            {
                ImGui::Columns(3, "##triggerpreferences", false);
                ImGui::Checkbox("Zoomed only", &trigger_zoomed);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Don't trigger with unzoomed rifles");
                ImGui::Checkbox("Wait for sniper rifle charge", &trigger_charge);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Triggerbot waits until it has enough charge to kill");
                ImGui::Checkbox("Improve accuracy", &trigger_accuracy);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Improves triggerbot accuracy when aiming for specific hitbox. Recommended to use with sniper rifle/ambassador");
                ImGui::Text("Hitbox Mode");
                ImGui::Combo("##hitboxmodetrigger", &trigger_hitboxmode, "AUTO-HEAD\0AUTO-CLOSEST\0Head only\0");
                ImGui::NextColumn();
                ImGui::Text("Delay");
                ImGui::SliderFloat("##triggerdelay", &trigger_delay, 0.0f, 1.0f);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Triggerbot delay in seconds");
                ImGui::Text("Max distance");
                ImGui::SliderInt("##triggermaxdistance", &trigger_maxrange, 0, 4096.0f);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Max range for triggerbot\n 900-1100 range is efficient for scout/widowmaker engineer");
                ImGui::NextColumn();
                ImGui::Text("Triggerbot key mode");
                ImGui::Combo("##triggerkeymode", &trigger_key_mode, "DISABLED\0TRIGGERKEY\0REVERSE\0TOGGLE\0");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("DISABLED: triggerbot is always active\nTRIGGERKEY triggerbot is active when key is down\nREVERSE: triggerbot is disabled when key is down\nTOGGLE: pressing key toggles triggerbot");
                ImGui::Text("Bind cat_trigger_key to a key!");
                ImGui::TreePop();
            }
        }
        break;

        case 3:
        {
            if (ImGui::TreeNode("ESP"))
            {
                ImGui::BeginChild(10, ImVec2(0, 265));
                ImGui::Columns(3, "##ESPstuff", false); 
                ImGui::Checkbox("Enable ESP", &esp_enabled);
                ImGui::Text("Box");
                ImGui::Combo("##espobox", &esp_box, "NONE\0NORMAL\0CORNERS\0");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Draw a 2D box");
                ImGui::Text("Corner Size");
                ImGui::DragInt("##espboxcornersize", &esp_box_corner_size, 1, 0, 50);
                ImGui::Text("Health ESP");
                ImGui::Combo("##esphealth", &esp_health, "NONE\0TEXT\0HEALTHBAR\0BOTH\0");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Show enemy health");
                ImGui::Text("Expand Esp");
                ImGui::DragInt("##expandesp", &esp_expand, 1, 0, 200);
                ImGui::Text("Tracers");
                ImGui::Combo("##esptracers", &esp_tracers, "OFF\0CENTER\0BOTTOM\0");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Draws a line from the player to a position on your screen");
                ImGui::Text("Text position");
                ImGui::Combo("##esptextposition", &esp_text_position, "TOP RIGHT\0BOTTOM RIGHT\0CENTER\0ABOVE\0BELOW\0");
                ImGui::NextColumn();                
                ImGui::Checkbox("Legit Mode", &esp_legit);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Don't show invisible enemies\nHides invisable enemies with visibility enabled");
                ImGui::Checkbox("VisCheck", &esp_vischeck);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("ESP visibility check - makes enemy info behind walls darker, disable this if you get FPS drops");
                ImGui::Checkbox("Draw Bones", &esp_bones);
                ImGui::Checkbox("Building ESP", &esp_buildings);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Show buildings");
                ImGui::Checkbox("ESP Teammates", &esp_teammates);
                ImGui::Checkbox("Name ESP", &esp_name);
                ImGui::Checkbox("Class ESP", &esp_class);
                ImGui::Checkbox("Distance ESP", &esp_distance);
                ImGui::Checkbox("Conditions ESP", &esp_conds);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Show conditions");
                ImGui::Checkbox("Ubercharge ESP", &esp_ubercharge);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Show ubercharge percentage while players medigun is out");
                ImGui::Checkbox("Show weapon name", &esp_weapon);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Show which weapon the enemy is using");
                ImGui::NextColumn();
                ImGui::Checkbox("Powerup ESP", &esp_powerups);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Shows powerups a player is using");
                ImGui::Checkbox("Show tank", &esp_show_tank);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Show tanks in mvm");
                ImGui::Checkbox("Show weapon spawners", &esp_weapon_spawners);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("TF2C deathmatch weapon spawners");
                ImGui::Checkbox("Followbot ESP", &esp_followbot_id);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Show followbot ID");
                ImGui::Checkbox("ESP Local Player", &esp_local);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Shows local player ESP in thirdperson");
                ImGui::Text("Show sightlines");
                ImGui::Combo("##showsightlines", &esp_sightlines, "NONE\0SNIPER ONLY\0ALL\0");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Displays a line of where players are looking");
                ImGui::Checkbox("Entity ESP", &esp_entity);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Show entity info (debug)");
                ImGui::Checkbox("Model name ESP", &esp_model_name);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Model name esp (DEBUG ONLY)");
                ImGui::Checkbox("Entity ID", &esp_entity_id);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Used with Entity ESP. Shows entityID");
                ImGui::Checkbox("Spellbooks", &esp_spellbooks);
                ImGui::EndChild();
                if (ImGui::TreeNode("Colors"))
                {
                    ImGui::BeginChild(11, ImVec2(0, 50));
                    ImGui::PushItemWidth(150);
                    ImGui::DragInt("##redred", &esp_color_red_r, 1, 0, 255, "R:%.0f"); ImGui::SameLine();
                    ImGui::PushItemWidth(150);
                    ImGui::DragInt("##redgreen", &esp_color_red_g, 1, 0, 255, "G:%.0f"); ImGui::SameLine();
                    ImGui::PushItemWidth(150);
                    ImGui::DragInt("Color for red team", &esp_color_red_b, 1, 0, 255, "B:%.0f");
                    ImGui::PushItemWidth(150);
                    ImGui::DragInt("##blured", &esp_color_blue_r, 1, 0, 255, "R:%.0f"); ImGui::SameLine();
                    ImGui::PushItemWidth(150);
                    ImGui::DragInt("##blugreen", &esp_color_blue_g, 1, 0, 255, "G:%.0f"); ImGui::SameLine();
                    ImGui::PushItemWidth(150);
                    ImGui::DragInt("Color for blue team", &esp_color_blue_b, 1, 0, 255, "B:%.0f");
                    ImGui::EndChild();
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Item ESP"))
                {
                    ImGui::BeginChild(12, ImVec2(0, 50));
                    ImGui::Columns(3, "##itemesp", false);
                    ImGui::Checkbox("Item ESP", &esp_item);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Master Item ESP switch (health packs, etc.)");
                    ImGui::Checkbox("Show Adrenaline", &esp_item_adrenaline);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("TF2C adrenaline pills");
                    ImGui::Checkbox("Ammo packs", &esp_item_powerups);
                    ImGui::NextColumn();
                    ImGui::Checkbox("Health packs", &esp_item_health);
                    ImGui::Checkbox("Powerups", &esp_item_ammo);
                    ImGui::Checkbox("Dropped weapons", &esp_item_weapons);
                    ImGui::NextColumn();
                    ImGui::Checkbox("MvM money", &esp_money_red);
                    ImGui::Checkbox("Red MvM money", &esp_money);
                    ImGui::EndChild();
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Projectile ESP"))
                {
                    ImGui::BeginChild(13, ImVec2(0, 50));
                    ImGui::Columns(3, "##projesp", false);
                    ImGui::Checkbox("Projectile ESP", &esp_proj);
                    ImGui::Checkbox("Only enemy projectiles", &esp_proj_enemy);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Don't show friendly projectiles");
                    ImGui::NextColumn();
                    ImGui::Text("Stickies");
                    ImGui::Combo("##projstickies", &esp_proj_stickies, "OFF\0ALL\0CRIT\0");
                    ImGui::Text("Pipes");
                    ImGui::Combo("##projstickies", &esp_proj_pipes, "OFF\0ALL\0CRIT\0");
                    ImGui::NextColumn();
                    ImGui::Text("Arrows");
                    ImGui::Combo("##projstickies", &esp_proj_arrows, "OFF\0ALL\0CRIT\0");
                    ImGui::Text("Rockets");
                    ImGui::Combo("##projstickies", &esp_proj_rockets, "OFF\0ALL\0CRIT\0");
                    ImGui::EndChild();
                    ImGui::TreePop();
                }
                //there should be radar here but it's broken and disabled, will be added in the future if needed
                if (ImGui::TreeNode("Chams"))
                {
                    ImGui::BeginChild(14, ImVec2(0,100));
                    ImGui::Columns(3, "##chamslol", false);
                    ImGui::Checkbox("Enable Chams", &chams_enable);
                    ImGui::Checkbox("Legit chams", &chams_legit);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Don't show chams through walls");
                    ImGui::Checkbox("Single-pass", &chams_singlepass);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Render chams only once (this disables 'darker' chams on invisible parts of player");
                    ImGui::Checkbox("Health", &chams_health);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Change chams color based on their health");
                    ImGui::Checkbox("Players", &chams_players);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Render chams on player models");
                    ImGui::NextColumn();
                    ImGui::Checkbox("Teammates", &chams_teammates);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Render chams on teammates");
                    ImGui::Checkbox("Buildings", &chams_buildings);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Render chams on buildings");
                    ImGui::Checkbox("Teammate Buildings", &chams_teammate_buildings);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Render chams on teammates buildings");
                    ImGui::Checkbox("Flat", &chams_flat);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Makes chams brighter and more full");
                    ImGui::Checkbox("Recursive", &chams_recursive);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Render chams on weapons and cosmetics");
                    ImGui::NextColumn();
                    ImGui::Checkbox("White Weapons", &chams_weapons_white);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Should chams on weapons be white");
                    ImGui::Checkbox("Medkits", &chams_medkits);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Render chams on medkits");
                    ImGui::Checkbox("Ammoboxes", &chams_ammo);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Render chams on ammoboxes");
                    ImGui::Checkbox("Stickies", &chams_stickies);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Render chams on stickybombs");
                    ImGui::EndChild();
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Glow"))
                {
                    ImGui::BeginChild(15, ImVec2(0, 100));
                    ImGui::Columns(3, "##glowlol", false);
                    ImGui::EndChild();
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }


        }
        break;

        case 4:
        {

        }
        break;

        case 5:
        {

        }
        break;

        case 6:
        {

        }
        break;

        case 7:
        {

        }
        break;
    }
    setvalues();
    ImGui::End();
}

void setvalues()
{
            //aimbot
            //checkboxes
    checkbox("cat_aimbot_enabled", aimbot, enable_var);
    checkbox("cat_aimbot_autoshoot", aimbot_autoshoot, enable_var1);
    checkbox("cat_aimbot_auto_unzoom", aimbot_auto_unzoom, enable_var2);
    checkbox("cat_aimbot_auto_zoom", aimbot_auto_zoom, enable_var3);
    checkbox("cat_aimbot_buildings_other", aimbot_buildings_other, enable_var4);
    checkbox("cat_aimbot_buildings_sentry", aimbot_buildings_sentry, enable_var5);
    checkbox("cat_aimbot_charge", aimbot_charge, enable_var6);
    checkbox("cat_aimbot_fov_draw", aimbot_fov_draw, enable_var7);
    checkbox("cat_aimbot_ignore_cloak", aimbot_ignore_cloak, enable_var8);
    checkbox("cat_aimbot_ignore_deadringer", aimbot_ignore_deadringer, enable_var9);
    checkbox("cat_aimbot_ignore_hoovy", aimbot_ignore_hoovy, enable_var10);
    checkbox("cat_aimbot_ignore_vaccinator", aimbot_ignore_vaccinator, enable_var11);
    checkbox("cat_aimbot_only_when_can_shoot", aimbot_only_when_can_shoot, enable_var12);
    checkbox("cat_aimbot_projectile", aimbot_projectile, enable_var13);
    checkbox("cat_aimbot_silent", aimbot_silent, enable_var14);
    checkbox("cat_aimbot_stickys", aimbot_stickys, enable_var15);
    checkbox("cat_aimbot_target_lock", aimbot_target_lock, enable_var16);
    checkbox("cat_aimbot_zoomed", aimbot_zoomed, enable_var17);
    checkbox("cat_autoheal_enabled", autoheal_enabled, enable_var18);
    checkbox("cat_autoheal_share_uber", autoheal_share_uber, enable_var19);
    checkbox("cat_autoheal_silent", autoheal_silent, enable_var20);
    checkbox("cat_autoheal_uber", autoheal_uber, enable_var21);
    checkbox("cat_ignore_taunting", ignore_taunting_sec, enable_var22);
            //combos/ints
    changeint("cat_aimbot_hitbox", aimbot_hitbox, aimbot_hitbox_check);
    changeint("cat_aimbot_hitboxmode", aimbot_hitboxmode, aimbot_hitboxmode_check);
    changeint("cat_aimbot_prioritymode", aimbot_prioritymode, aimbot_prioritymode_check);
    changeint("cat_aimbot_teammates", aimbot_teammates, aimbot_teammates_check);
    changeint("cat_aimbot_fov", aimbot_fov, aimbot_fov_check);
    changeint("cat_aimbot_maxrange", aimbot_maxrange, aimbot_maxrange_check);
    changeint("cat_aimbot_proj_speed", aimbot_proj_speed, aimbot_proj_speed_check);
    changeint("cat_autoheal_uber_health", autoheal_uber_health, autoheal_uber_health_check);
            //floats
    changefloat("cat_aimbot_fov_draw_opacity", aimbot_fov_draw_opacity, aimbot_fov_draw_opacity_check);
    changefloat("cat_aimbot_slow", aimbot_slow, aimbot_slow_check);
    changefloat("cat_aimbot_huntsman_charge", aimbot_huntsman_charge, aimbot_huntsman_charge_check);
    changefloat("cat_aimbot_miss_chance", aimbot_miss_chance, aimbot_miss_chance_check);
    //triggerbot
            //checkboxes
    checkbox("cat_autobackstab", autobackstab, enable_2var);
    checkbox("cat_auto_vacc", auto_vacc, enable_2var1);
    checkbox("cat_auto_vacc_afterburn", auto_vacc_afterburn, enable_2var2);
    checkbox("cat_auto_vacc_blast", auto_vacc_blast, enable_2var3);
    checkbox("cat_auto_vacc_blast_pop_crit", auto_vacc_blast_pop_crit, enable_2var4);
    checkbox("cat_auto_vacc_bullets", auto_vacc_bullets, enable_2var5);
    checkbox("cat_auto_vacc_fire", auto_vacc_fire, enable_2var6);
    checkbox("cat_reflect_dodgeball", reflect_dodgeball, enable_2var7);
    checkbox("cat_reflect_enabled", reflect_enabled, enable_2var8);
    checkbox("cat_reflect_legit", reflect_legit, enable_2var9);
    checkbox("cat_reflect_only_idle", reflect_only_idle, enable_2var10);
    checkbox("cat_reflect_stickybombs", reflect_stickybombs, enable_2var11);
    checkbox("cat_sticky_buildings", sticky_buildings, enable_2var12);
    checkbox("cat_sticky_enabled", sticky_enabled, enable_2var13);
    checkbox("cat_sticky_legit", sticky_legit, enable_2var14);
    checkbox("cat_trigger_accuracy", trigger_accuracy, enable_2var15);
    checkbox("cat_trigger_buildings_other", trigger_buildings_other, enable_2var16);
    checkbox("cat_trigger_buildings_sentry", trigger_buildings_sentry, enable_2var17);
    checkbox("cat_trigger_charge", trigger_charge, enable_2var18);
    checkbox("cat_trigger_enabled", trigger_enabled, enable_2var19);
    checkbox("cat_trigger_ignore_cloak", trigger_ignore_cloak, enable_2var20);
    checkbox("cat_trigger_ignore_hoovy", trigger_ignore_hoovy, enable_2var21);
    checkbox("cat_trigger_ignore_vaccinator", trigger_ignore_vaccinator, enable_2var22);
    checkbox("cat_trigger_stickys", trigger_stickys, enable_2var23);
    checkbox("cat_trigger_teammates", trigger_teammates, enable_2var24);
    checkbox("cat_trigger_zoomed", trigger_zoomed, enable_2var25);
            //combos/ints
    changeint("cat_auto_vacc_default_resist", auto_vacc_default_resist, auto_vacc_default_resist_check);
    changeint("cat_auto_vacc_fire_pop_pyro", auto_vacc_fire_pop_pyro, auto_vacc_fire_pop_pyro_check);
    changeint("cat_auto_vacc_sniper_pop", auto_vacc_sniper_pop, auto_vacc_sniper_pop_check);
    changeint("cat_trigger_hitboxmode", trigger_hitboxmode, trigger_hitboxmode_check);
    changeint("cat_trigger_key_mode", trigger_key_mode, trigger_key_mode_check);
    changeint("cat_auto_vacc_blast_pop_health", auto_vacc_blast_pop_health, auto_vacc_blast_pop_health_check);
    changeint("cat_auto_vacc_blast_pop_ubers", auto_vacc_blast_pop_ubers, auto_vacc_blast_pop_ubers_check);
    changeint("cat_auto_vacc_bullet_pop_ubers", auto_vacc_bullet_pop_ubers, auto_vacc_bullet_pop_ubers_check);
    changeint("cat_auto_vacc_fire_pop_ubers", auto_vacc_fire_pop_ubers, auto_vacc_fire_pop_ubers_check);
    changeint("cat_auto_vacc_pyro_range", auto_vacc_pyro_range, auto_vacc_pyro_range_check);
    changeint("cat_auto_vacc_reset_timer", auto_vacc_reset_timer, auto_vacc_reset_timer_check);
    changeint("cat_auto_vacc_rocket_range", auto_vacc_rocket_range, auto_vacc_rocket_range_check);
    changeint("cat_trigger_maxrange", trigger_maxrange, trigger_maxrange_check);
            //floats
    changefloat("cat_trigger_delay", trigger_delay, trigger_delay_check);
    //visual/esp
            //checkboxes
    checkbox("cat_esp_enabled", esp_enabled, enable_3var);
    checkbox("cat_esp_legit", esp_legit, enable_3var1);
    checkbox("cat_esp_vischeck", esp_vischeck, enable_3var2);
    checkbox("cat_esp_bones", esp_bones, enable_3var3);
    checkbox("cat_esp_buildings", esp_buildings, enable_3var4);
    checkbox("cat_esp_teammates", esp_teammates, enable_3var5);
    checkbox("cat_esp_name", esp_name, enable_3var6);
    checkbox("cat_esp_class", esp_class, enable_3var7);
    checkbox("cat_esp_distance", esp_distance, enable_3var8);
    checkbox("cat_esp_conds", esp_conds, enable_3var9);
    checkbox("cat_esp_ubercharge", esp_ubercharge, enable_3var10);
    checkbox("cat_esp_weapon", esp_weapon, enable_3var11);
    checkbox("cat_esp_powerups", esp_powerups, enable_3var12);
    checkbox("cat_esp_show_tank", esp_show_tank, enable_3var13);
    checkbox("cat_esp_weapon_spawners", esp_weapon_spawners, enable_3var14);
    checkbox("cat_esp_followbot_id", esp_followbot_id, enable_3var15);
    checkbox("cat_esp_local", esp_local, enable_3var16);
    checkbox("cat_esp_entity", esp_entity, enable_3var17);
    checkbox("cat_esp_model_name", esp_model_name, enable_3var18);
    checkbox("cat_esp_entity_id", esp_entity_id, enable_3var19);
    checkbox("cat_esp_spellbooks", autobackstab, enable_3var20);
            //combos/ints
    changeint("cat_esp_box", esp_box, esp_box_check);
    changeint("cat_esp_box_corner_size", esp_box_corner_size, esp_box_corner_size_check);
    changeint("cat_esp_health", esp_health, esp_health_check);
    changeint("cat_esp_tracers", esp_tracers, esp_tracers_check);
    changeint("cat_esp_text_position", esp_text_position, esp_text_position_check);
    changeint("cat_esp_sightlines", esp_sightlines, esp_sightlines_check);
    changeint("cat_esp_expand", esp_expand, esp_expand_check);
}

void checkbox (const char* varname, bool &variable, int &enable_var)
{
    if (variable)
    {
        if (!enable_var)
        {
            g_pCVar->FindVar(varname)->SetValue(1);
            enable_var = 1;
        }
    }
    else
    {
        if (enable_var)
        {
            g_pCVar->FindVar(varname)->SetValue(0);
            enable_var = 0;
        }
    }
    ConVar* variable_var = g_pCVar->FindVar(varname);
    if(variable_var->GetBool())
    {
        variable = true;
    }
    else
    {
        variable = false;
    }
}

void changeint (const char* varname, int &variable, int variable_check)
{
    if (variable != variable_check)
    {   
        g_pCVar->FindVar(varname)->SetValue(variable);
    }
    else
    {   
        ConVar* variable_var = g_pCVar->FindVar(varname);
        int variable_int = (variable_var->GetInt());
        if(variable_int != variable)
        {
            variable = variable_int;
        }
    }
}

void changefloat (const char* varname, float &variable, float variable_check)
{
    if (variable != variable_check)
    {   
        g_pCVar->FindVar(varname)->SetValue(variable);
    }
    else
    {   
        ConVar* variable_var = g_pCVar->FindVar(varname);
        float variable_float = (variable_var->GetFloat());
        if(variable_float != variable)
        {
            variable = variable_float;
        }
    }
}