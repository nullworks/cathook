#include "common.hpp"
#include "imgui/imgui.h"
int tabs = 1;


bool aimbot = false;
int aimkey_mode = 1;
int aimkey_mode_num = 2;
bool aimbot_autoshoot = true;
int aimbot_hitboxmode = 0;
int aimbot_hitboxmode_num = 2;
float aimbot_fov = 0;
bool aimbot_fov_draw = 0;
float aimbot_fov_draw_opacity = 0.7f;
int aimbot_prioritymode = 0;
int aimbot_prioritymode_num = 2;
bool aimbot_charge = false;
bool aimbot_ignore_hoovy = false;
bool aimbot_ignore_cloak = true;
bool aimbot_ignore_deadringer = true;
bool aimbot_buildings_sentry = true;
bool aimbot_buildings_other = true;
bool aimbot_stickys = false;
bool aimbot_rage_only = false;
int aimbot_teammates = 0;
int aimbot_teammates_num = 2;
bool aimbot_auto_unzoom = false;
bool aimbot_auto_zoom = false;
bool ignore_taunting_sec = true;
bool aimbot_ignore_vaccinator = true;

void menu()
{
	ImGui::Begin("Config Menu");
    if (ImGui::Button("Aimbot##tab"))
    {
        tabs = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Wallhack"))
    {
        tabs = 2;
    }    
    switch (tabs)
    {
        case 1:
        {
            ImGui::BeginChild(1, ImVec2(0,120), false);
            ImGui::Columns(2);
            ImGui::Checkbox("Enable Aimbot", &aimbot);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Main aimbot switch");
            ImGui::Checkbox("Enable Autoshoot", &aimbot_autoshoot);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Shoot automatically when the target is locked, isn't compatible with 'Enable when attacking'");
            ImGui::Checkbox("Wait for sniper rifle charge", &aimbot_charge);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Aimbot waits until it has enough charge to kill");
            if (ImGui::Button("Priority mode"))
            {
                aimbot_prioritymode++;
                aimbot_prioritymode_num++;
                if (aimbot_prioritymode > 3)
                {
                    aimbot_prioritymode = 0;
                }
            }
            ImGui::SameLine();
            switch (aimbot_prioritymode)
            {
                case 0: ImGui::Text("SMART");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Basically Auto-Threat.");
            break;
                case 1: ImGui::Text("FOV");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("¯\_(ツ)_/¯");
            break;
                case 2: ImGui::Text("DISTANCE"); break;
                case 3: ImGui::Text("HEALTH"); break;
            }
            ImGui::SliderFloat("Aimbot FOV", &aimbot_fov, 0, 60);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("FOV range for aimbot to lock targets.");
            ImGui::NextColumn();
            ImGui::Checkbox("Auto zoom", &aimbot_auto_zoom);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Automatically zoom");
            ImGui::Checkbox("Auto Un-zoom", &aimbot_auto_unzoom);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Automatically unzoom");
            if (ImGui::Button("HItbox Mode"))
            {
                aimbot_hitboxmode++;
                aimbot_hitboxmode_num++;
                if (aimbot_hitboxmode > 2)
                {
                    aimbot_hitboxmode = 0;
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Defines hitbox selection mode");
            ImGui::SameLine();
            switch (aimbot_hitboxmode)
            {
                case 0: ImGui::Text("AUTO"); break;
                case 1: ImGui::Text("AUTO-CLOSEST"); break;
                case 2: ImGui::Text("STATIC"); break;
            }
            ImGui::Checkbox("Draw Fov Ring", &aimbot_fov_draw);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Draws a ring to represent your current aimbot fov");
            ImGui::SliderFloat("FOV Circle Opacity", &aimbot_fov_draw_opacity, 0.0f, 1.0f);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Defines opacity of FOV circle");
            ImGui::EndChild();
            if (ImGui::TreeNode("Ignore"))
            {
                ImGui::BeginChild(2, ImVec2(0,70), false);
                ImGui::Columns(3);
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
                if (ImGui::Button("Aim at teammates"))
                {
                    aimbot_teammates++;
                    aimbot_teammates_num++;
                    if (aimbot_teammates > 2)
                    {
                        aimbot_teammates = 0;
                    }
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Use to choose which team/s to target");
                ImGui::SameLine();
                switch (aimbot_teammates)
                {
                    case 0: ImGui::Text("ENEMY ONLY"); break;
                    case 1: ImGui::Text("TEAMMATE ONLY"); break;
                    case 2: ImGui::Text("BOTH"); break;
                }
                ImGui::Checkbox("Aim Sticky", &aimbot_stickys);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Should aimbot aim at stickys");
                ImGui::EndChild();
                ImGui::TreePop();
            }
            ImGui::Checkbox("Aim Sentry", &aimbot_buildings_sentry);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Should aimbot aim at sentryguns");
            
            ImGui::Checkbox("Ignore non-rage targets", &aimbot_rage_only);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Use playerlist to set up rage targets, probably broken");
            
            if (ImGui::Button("Aimkey Mode"))
            {
                aimkey_mode++;
                aimkey_mode_num++;
                if (aimkey_mode > 4)
                {
                    aimkey_mode = 1;
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Bind cat_aimbot_aimkey first!");
            ImGui::SameLine();
            switch (aimkey_mode)
            {
                case 1: ImGui::Text("DISABLED");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("aimbot is always active");
            break;
                case 2: ImGui::Text("AIMKEY");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Aimbot is active when key is down");
            break;
                case 3: ImGui::Text("REVERSE");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Aimbot is disabled when key is down");
            break;
                case 4: ImGui::Text("TOGGLE");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Pressing key toggles aimbot");
            break;
            }
        }
        break;
        
        case 2:
        {
            ImGui::Text("WIP LOL");
        }
        break;
    }

    ImGui::End();
}