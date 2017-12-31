/*
 * HEsp.cpp
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"

namespace hacks
{
namespace shared
{
namespace esp
{

// Main Switch
CatVar enabled(CV_SWITCH, "esp_enabled", "0", "ESP", "Master ESP switch");
// Box esp + Options
CatEnum box_esp_enum({ "None", "Normal", "Corners" });
CatVar box_esp(box_esp_enum, "esp_box", "2", "Box", "Draw a 2D box");
CatVar box_corner_size(CV_INT, "esp_box_corner_size", "10", "Corner Size");
// Tracers
CatEnum tracers_enum({ "OFF", "CENTER", "BOTTOM" });
CatVar tracers(tracers_enum, "esp_tracers", "0", "Tracers",
               "SDraws a line from the player to a position on your screen");
// Emoji Esp
#ifndef FEATURE_EMOJI_ESP_DISABLED
CatEnum emoji_esp_enum({ "None", "Joy", "Thinking" });
CatVar emoji_esp(emoji_esp_enum, "esp_emoji", "0", "Emoji ESP",
                 "Draw emoji on peopels head");
CatVar emoji_esp_size(CV_FLOAT, "esp_emoji_size", "32", "Emoji ESP Size");
CatVar emoji_esp_scaling(CV_SWITCH, "esp_emoji_scaling", "1",
                         "Emoji ESP Scaling");
CatVar emoji_min_size(CV_INT, "esp_emoji_min_size", "20", "Emoji ESP min size",
                      "Minimum size for an emoji when you use auto scaling");
#endif
// Other esp options
CatEnum show_health_enum({ "None", "Text", "Healthbar", "Both" });
CatVar show_health(show_health_enum, "esp_health", "3", "Health ESP",
                   "Show enemy health");
CatVar draw_bones(CV_SWITCH, "esp_bones", "0", "Draw Bones");
CatEnum sightlines_enum({ "None", "Sniper Only",
                          "All" }); // I ripped of lbox's choices cuz its nice
CatVar sightlines(sightlines_enum, "esp_sightlines", "0", "Show sightlines",
                  "Displays a line of where players are looking");
CatEnum esp_text_position_enum({ "TOP RIGHT", "BOTTOM RIGHT", "CENTER", "ABOVE",
                                 "BELOW" });
CatVar esp_text_position(esp_text_position_enum, "esp_text_position", "0",
                         "Text position", "Defines text position");
CatVar esp_expand(
    CV_INT, "esp_expand", "0", "Expand Esp",
    "Spreads out Box, health bar, and text from center"); // Note, check if this
                                                          // should be int, it
                                                          // is being used by
                                                          // casting as float
CatVar vischeck(CV_SWITCH, "esp_vischeck", "1", "VisCheck",
                "ESP visibility check - makes enemy info behind walls darker, "
                "disable this if you get FPS drops");
CatVar legit(CV_SWITCH, "esp_legit", "0", "Legit Mode",
             "Don't show invisible enemies\nHides invisable enemies with "
             "visibility enabled");
// Selective esp options
CatVar local_esp(CV_SWITCH, "esp_local", "1", "ESP Local Player",
                 "Shows local player ESP in thirdperson");
CatVar buildings(CV_SWITCH, "esp_buildings", "1", "Building ESP",
                 "Show buildings");
CatVar teammates(CV_SWITCH, "esp_teammates", "0", "ESP Teammates",
                 "Teammate ESP");
CatVar tank(CV_SWITCH, "esp_show_tank", "1", "Show tank", "Show tanks in mvm");
// Text Esps
CatVar show_weapon(CV_SWITCH, "esp_weapon", "0", "Show weapon name",
                   "Show which weapon the enemy is using");
CatVar show_distance(CV_SWITCH, "esp_distance", "1", "Distance ESP",
                     "Show distance to target");
CatVar show_name(CV_SWITCH, "esp_name", "1", "Name ESP", "Show name");
CatVar show_class(CV_SWITCH, "esp_class", "1", "Class ESP", "Show class");
CatVar show_conditions(CV_SWITCH, "esp_conds", "1", "Conditions ESP",
                       "Show conditions");
CatVar
    show_ubercharge(CV_SWITCH, "esp_ubercharge", "1", "Ubercharge ESP",
                    "Show ubercharge percentage while players medigun is out");
CatVar show_bot_id(CV_SWITCH, "esp_followbot_id", "1", "Followbot ESP",
                   "Show followbot ID");
CatVar powerup_esp(CV_SWITCH, "esp_powerups", "1", "Powerup ESP",
                   "Shows powerups a player is using");
// Item esp
CatVar item_esp(CV_SWITCH, "esp_item", "1", "Item ESP",
                "Master Item ESP switch (health packs, etc.)");
CatVar item_dropped_weapons(CV_SWITCH, "esp_item_weapons", "0",
                            "Dropped weapons", "Show dropped weapons");
CatVar item_ammo_packs(CV_SWITCH, "esp_item_ammo", "0", "Ammo packs",
                       "Show ammo packs");
CatVar item_health_packs(CV_SWITCH, "esp_item_health", "1", "Health packs",
                         "Show health packs");
CatVar item_powerups(CV_SWITCH, "esp_item_powerups", "1", "Powerups",
                     "Shows powerups in the world");
CatVar item_money(CV_SWITCH, "esp_money", "1", "MvM money", "Show MvM money");
CatVar item_money_red(CV_SWITCH, "esp_money_red", "1", "Red MvM money",
                      "Show red MvM money");
CatVar item_spellbooks(CV_SWITCH, "esp_spellbooks", "1", "Spellbooks",
                       "Spell Books");
CatVar item_weapon_spawners(CV_SWITCH, "esp_weapon_spawners", "1",
                            "Show weapon spawners",
                            "TF2C deathmatch weapon spawners");
CatVar item_adrenaline(CV_SWITCH, "esp_item_adrenaline", "0", "Show Adrenaline",
                       "TF2C adrenaline pills");
// Projectile esp
CatVar proj_esp(CV_SWITCH, "esp_proj", "1", "Projectile ESP", "Projectile ESP");
CatEnum proj_esp_enum({ "OFF", "ALL", "CRIT" });
CatVar proj_rockets(proj_esp_enum, "esp_proj_rockets", "1", "Rockets",
                    "Rockets");
CatVar proj_arrows(proj_esp_enum, "esp_proj_arrows", "1", "Arrows", "Arrows");
CatVar proj_pipes(proj_esp_enum, "esp_proj_pipes", "1", "Pipes", "Pipebombs");
CatVar proj_stickies(proj_esp_enum, "esp_proj_stickies", "1", "Stickies",
                     "Stickybombs");
CatVar proj_enemy(CV_SWITCH, "esp_proj_enemy", "1", "Only enemy projectiles",
                  "Don't show friendly projectiles");
// Debug
CatVar entity_info(CV_SWITCH, "esp_entity", "0", "Entity ESP",
                   "Show entity info (debug)");
CatVar entity_model(CV_SWITCH, "esp_model_name", "0", "Model name ESP",
                    "Model name esp (DEBUG ONLY)");
CatVar entity_id(CV_SWITCH, "esp_entity_id", "1", "Entity ID",
                 "Used with Entity ESP. Shows entityID");

// CatVar draw_hitbox(CV_SWITCH, "esp_hitbox", "1", "Draw Hitbox");

// Unknown
std::mutex threadsafe_mutex;
// Storage array for keeping strings and other data
std::array<ESPData, 2048> data;

// Storage vars for entities that need to be re-drawn
std::vector<int> entities_need_repaint{};
std::mutex entities_need_repaint_mutex{};

// :b:one stuff needs to be up here as puting it in the header for sorting would
// be a pain.

// Vars to store what bones connect to what
const std::string bonenames_leg_r[] = { "bip_foot_R", "bip_knee_R",
                                        "bip_hip_R" };
const std::string bonenames_leg_l[] = { "bip_foot_L", "bip_knee_L",
                                        "bip_hip_L" };
const std::string bonenames_bottom[] = { "bip_hip_R", "bip_pelvis",
                                         "bip_hip_L" };
const std::string bonenames_spine[] = { "bip_pelvis",  "bip_spine_0",
                                        "bip_spine_1", "bip_spine_2",
                                        "bip_spine_3", "bip_neck",
                                        "bip_head" };
const std::string bonenames_arm_r[] = { "bip_upperArm_R", "bip_lowerArm_R",
                                        "bip_hand_R" };
const std::string bonenames_arm_l[] = { "bip_upperArm_L", "bip_lowerArm_L",
                                        "bip_hand_L" };
const std::string bonenames_up[] = { "bip_upperArm_R", "bip_spine_3",
                                     "bip_upperArm_L" };

// Dont fully understand struct but a guess is a group of something.
// I will return once I have enough knowlage to reverse this.
// NOTE: No idea on why we cant just use gethitbox and use the displacement on
// that insted of having all this extra code. Shouldnt gethitbox use cached
// hitboxes, if so it should be nicer on performance
struct bonelist_s
{
    bool setup{ false };
    bool success{ false };
    int leg_r[3]{ 0 };
    int leg_l[3]{ 0 };
    int bottom[3]{ 0 };
    int spine[7]{ 0 };
    int arm_r[3]{ 0 };
    int arm_l[3]{ 0 };
    int up[3]{ 0 };

    void Setup(const studiohdr_t *hdr)
    {
        if (!hdr)
        {
            setup = true;
            return;
        }
        std::unordered_map<std::string, int> bones{};
        for (int i = 0; i < hdr->numbones; i++)
        {
            bones[std::string(hdr->pBone(i)->pszName())] = i;
        }
        try
        {
            for (int i   = 0; i < 3; i++)
                leg_r[i] = bones.at(bonenames_leg_r[i]);
            for (int i   = 0; i < 3; i++)
                leg_l[i] = bones.at(bonenames_leg_l[i]);
            for (int i    = 0; i < 3; i++)
                bottom[i] = bones.at(bonenames_bottom[i]);
            for (int i   = 0; i < 7; i++)
                spine[i] = bones.at(bonenames_spine[i]);
            for (int i   = 0; i < 3; i++)
                arm_r[i] = bones.at(bonenames_arm_r[i]);
            for (int i   = 0; i < 3; i++)
                arm_l[i] = bones.at(bonenames_arm_l[i]);
            for (int i = 0; i < 3; i++)
                up[i]  = bones.at(bonenames_up[i]);
            success    = true;
        }
        catch (std::exception &ex)
        {
            logging::Info("Bone list exception: %s", ex.what());
        }
        setup = true;
    }

    void DrawBoneList(const matrix3x4_t *bones, int *in, int size,
                      const rgba_t &color, const Vector &displacement)
    {
        Vector last_screen;
        Vector current_screen;
        for (int i = 0; i < size; i++)
        {
            Vector position(bones[in[i]][0][3], bones[in[i]][1][3],
                            bones[in[i]][2][3]);
            position += displacement;
            if (!draw::WorldToScreen(position, current_screen))
            {
                return;
            }
            if (i > 0)
            {
                draw_api::draw_line(last_screen.x, last_screen.y,
                                    current_screen.x - last_screen.x,
                                    current_screen.y - last_screen.y, color,
                                    0.5f);
            }
            last_screen = current_screen;
        }
    }

    void Draw(CachedEntity *ent, const rgba_t &color)
    {
        const model_t *model = RAW_ENT(ent)->GetModel();
        if (not model)
        {
            return;
        }

        studiohdr_t *hdr = g_IModelInfo->GetStudiomodel(model);

        if (!setup)
        {
            Setup(hdr);
        }
        if (!success)
            return;

        // ent->m_bBonesSetup = false;
        Vector displacement = RAW_ENT(ent)->GetAbsOrigin() - ent->m_vecOrigin;
        const auto &bones   = ent->hitboxes.GetBones();
        DrawBoneList(bones, leg_r, 3, color, displacement);
        DrawBoneList(bones, leg_l, 3, color, displacement);
        DrawBoneList(bones, bottom, 3, color, displacement);
        DrawBoneList(bones, spine, 7, color, displacement);
        DrawBoneList(bones, arm_r, 3, color, displacement);
        DrawBoneList(bones, arm_l, 3, color, displacement);
        DrawBoneList(bones, up, 3, color, displacement);
        /*for (int i = 0; i < hdr->numbones; i++) {
            const auto& bone = ent->GetBones()[i];
            Vector pos(bone[0][3], bone[1][3], bone[2][3]);
            //pos += orig;
            Vector screen;
            if (draw::WorldToScreen(pos, screen)) {
                if (hdr->pBone(i)->pszName()) {
                    draw::FString(fonts::ESP, screen.x, screen.y, fg, 2, "%s
        [%d]", hdr->pBone(i)->pszName(), i); } else draw::FString(fonts::ESP,
        screen.x, screen.y, fg, 2, "%d", i);
            }
        }*/
    }
};

std::unordered_map<studiohdr_t *, bonelist_s> bonelist_map{};

// Function called on draw
void Draw()
{
    std::lock_guard<std::mutex> esp_lock(threadsafe_mutex);
    if (!enabled)
        return;
    for (auto &i : entities_need_repaint)
    {
        ProcessEntityPT(ENTITY(i));
    }
}

// Function called on create move
void CreateMove()
{

    // Check usersettings if enabled
    if (!enabled)
        return;

    // Something
    std::lock_guard<std::mutex> esp_lock(threadsafe_mutex);

    ResetEntityStrings();          // Clear any strings entities have
    entities_need_repaint.clear(); // Clear data on entities that need redraw

    // TODO, Find the benefit of using max clients with this logic
    // Get max clients and
    static int max_clients = g_IEngine->GetMaxClients();
    int limit              = HIGHEST_ENTITY;

    // If not using any other special esp, we lower the min to the max clients
    if (!buildings && !proj_esp && !item_esp)
        limit = std::min(max_clients, HIGHEST_ENTITY);

    { // I still dont understand the purpose of prof_section and surrounding in
        // brackets
        PROF_SECTION(CM_ESP_EntityLoop);

        // Loop through entities
        for (int i = 0; i < limit; i++)
        {
            // Get an entity from the loop tick and process it
            CachedEntity *ent = ENTITY(i);
            ProcessEntity(ent);

            // Dont know what this check is for
            if (data[i].string_count)
            {

                // Set entity color
                SetEntityColor(ent, colors::EntityF(ent));

                // If snow distance, add string here
                if (show_distance)
                {
                    AddEntityString(ent, format((int) (ENTITY(i)->m_flDistance /
                                                       64 * 1.22f),
                                                'm'));
                }
            }
            // No idea, this is confusing
            if (data[ent->m_IDX].needs_paint)
                entities_need_repaint.push_back(ent->m_IDX);
        }
    }
}

// Used when processing entitys with cached data from createmove in draw
void _FASTCALL ProcessEntityPT(CachedEntity *ent)
{
    PROF_SECTION(PT_esp_process_entity);

    // Check to prevent crashes
    if (CE_BAD(ent))
        return;

    // Grab esp data
    ESPData &ent_data = data[ent->m_IDX];

    // Get color of entity
    // TODO, check if we can move this after world to screen check
    rgba_t fg = ent_data.color;
    if (!fg || fg.a == 0.0f)
        fg = ent_data.color = colors::EntityF(ent);

    // Check if entity is on screen, then save screen position if true
    Vector screen, origin_screen;
    if (!draw::EntityCenterToScreen(ent, screen) &&
        !draw::WorldToScreen(ent->m_vecOrigin, origin_screen))
        return;

    // Reset the collide cache
    ent_data.has_collide = false;

    // Get if ent should be transparent
    bool transparent = false;
    if (vischeck && !ent->IsVisible())
        transparent = true;

    // Bone esp
    if (draw_bones && ent->m_Type == ENTITY_PLAYER)
    {
        const model_t *model = RAW_ENT(ent)->GetModel();
        if (model)
        {
            auto hdr = g_IModelInfo->GetStudiomodel(model);
            bonelist_map[hdr].Draw(ent, fg);
        }
    }

    // Tracers
    if (tracers && ent->m_Type == ENTITY_PLAYER)
    {

        // Grab the screen resolution and save to some vars
        int width, height;
        g_IEngine->GetScreenSize(width, height);

        // Center values on screen
        width = width / 2;
        // Only center height if we are using center mode
        if ((int) tracers == 1)
            height = height / 2;

        // Get world to screen
        Vector scn;
        draw::WorldToScreen(ent->m_vecOrigin, scn);

        // Draw a line
        draw_api::draw_line(scn.x, scn.y, width - scn.x, height - scn.y, fg,
                            0.5f);
    }

    // Sightline esp
    if (sightlines && ent->m_Type == ENTITY_PLAYER)
    {

        // Logic for using the enum to sort out snipers
        if ((int) sightlines == 2 ||
            ((int) sightlines == 1 && CE_INT(ent, netvar.iClass) == tf_sniper))
        {
            PROF_SECTION(PT_esp_sightlines);

            // Get players angle and head position
            Vector &eye_angles =
                NET_VECTOR(RAW_ENT(ent), netvar.m_angEyeAngles);
            Vector eye_position;
            GetHitbox(ent, 0, eye_position);

            // Main ray tracing area
            float sy         = sinf(DEG2RAD(eye_angles.y)); // yaw
            float cy         = cosf(DEG2RAD(eye_angles.y));
            float sp         = sinf(DEG2RAD(eye_angles.x)); // pitch
            float cp         = cosf(DEG2RAD(eye_angles.x));
            Vector forward_t = Vector(cp * cy, cp * sy, -sp);
            // We dont want the sightlines endpoint to go behind us because the
            // world to screen check will fail, but keep it at most 4096
            Vector forward = forward_t * 4096.0F + eye_position;
            Ray_t ray;
            ray.Init(eye_position, forward);
            trace_t trace;
            g_ITrace->TraceRay(ray, MASK_SHOT_HULL, &trace::filter_no_player,
                               &trace);

            // Screen vectors
            Vector scn1, scn2;

            // Status vars
            bool found_scn2 = true;

            // Get end point on screen
            if (!draw::WorldToScreen(trace.endpos, scn2))
            {
                // Set status
                found_scn2 = false;
                // Get the end distance from the trace
                float end_distance = trace.endpos.DistTo(eye_position);

                // Loop and look back untill we have a vector on screen
                for (int i = 1; i > 400; i++)
                {
                    // Subtract 40 multiplyed by the tick from the end distance
                    // and use that as our length to check
                    Vector end_vector =
                        forward_t * (end_distance - (10 * i)) + eye_position;
                    if (end_vector.DistTo(eye_position) < 1)
                        break;
                    if (draw::WorldToScreen(end_vector, scn2))
                    {
                        found_scn2 = true;
                        break;
                    }
                }
            }

            if (found_scn2)
            {
                // Set status
                bool found_scn1 = true;

                // If we dont have a vector on screen, attempt to find one
                if (!draw::WorldToScreen(eye_position, scn1))
                {
                    // Set status
                    found_scn1 = false;
                    // Get the end distance from the trace
                    float start_distance = trace.endpos.DistTo(eye_position);

                    // Loop and look back untill we have a vector on screen
                    for (int i = 1; i > 400; i++)
                    {
                        // Multiply starting distance by 40, multiplyed by the
                        // loop tick
                        Vector start_vector =
                            forward_t * (10 * i) + eye_position;
                        // We dont want it to go too far
                        if (start_vector.DistTo(trace.endpos) < 1)
                            break;
                        // Check if we have a vector on screen, if we do then we
                        // set our status
                        if (draw::WorldToScreen(start_vector, scn1))
                        {
                            found_scn1 = true;
                            break;
                        }
                    }
                }
                // We have both vectors, draw
                if (found_scn1)
                {
                    draw_api::draw_line(scn1.x, scn1.y, scn2.x - scn1.x,
                                        scn2.y - scn1.y, fg, 0.5f);
                }
            }
        }
    }
#ifndef FEATURE_EMOJI_ESP_DISABLED
    // Emoji esp
    if (emoji_esp)
    {
        if (ent->m_Type == ENTITY_PLAYER)
        {
            // Positions in the atlas for the textures
            static textures::AtlasTexture joy_texture(
                64 * 4, textures::atlas_height - 64 * 4, 64, 64);
            static textures::AtlasTexture thinking_texture(
                64 * 5, textures::atlas_height - 64 * 4, 64, 64);

            auto hb = ent->hitboxes.GetHitbox(0);
            Vector hbm, hbx;
            if (draw::WorldToScreen(hb->min, hbm) &&
                draw::WorldToScreen(hb->max, hbx))
            {
                Vector head_scr;
                if (draw::WorldToScreen(hb->center, head_scr))
                {
                    float size = emoji_esp_scaling ? fabs(hbm.y - hbx.y)
                                                   : float(emoji_esp_size);
                    if (emoji_esp_scaling && (size < float(emoji_min_size)))
                    {
                        size = float(emoji_min_size);
                    }
                    textures::AtlasTexture *tx = nullptr;
                    if (int(emoji_esp) == 1)
                        tx = &joy_texture;
                    if (int(emoji_esp) == 2)
                        tx = &thinking_texture;
                    if (tx)
                        tx->Draw(head_scr.x - size / 2, head_scr.y - size / 2,
                                 colors::white, size, size);
                }
            }
        }
    }
#endif
    // Box esp
    if (box_esp)
    {
        switch (ent->m_Type)
        {
        case ENTITY_PLAYER:
            if (vischeck && !ent->IsVisible())
                transparent = true;
            if (!fg)
                fg = colors::EntityF(ent);
            if (transparent)
                fg = colors::Transparent(fg);
            DrawBox(ent, fg);
            break;
        case ENTITY_BUILDING:
            if (CE_INT(ent, netvar.iTeamNum) == g_pLocalPlayer->team &&
                !teammates)
                break;
            if (!transparent && vischeck && !ent->IsVisible())
                transparent = true;
            if (!fg)
                fg = colors::EntityF(ent);
            if (transparent)
                fg = colors::Transparent(fg);
            DrawBox(ent, fg);
            break;
        }
    }

    // Healthbar
    if ((int) show_health >= 2)
    {

        // We only want health bars on players and buildings
        if (ent->m_Type == ENTITY_PLAYER || ent->m_Type == ENTITY_BUILDING)
        {

            // Get collidable from the cache
            if (GetCollide(ent))
            {

                // Pull the cached collide info
                int max_x = ent_data.collide_max.x;
                int max_y = ent_data.collide_max.y;
                int min_x = ent_data.collide_min.x;
                int min_y = ent_data.collide_min.y;

                // Get health values
                int health    = 0;
                int healthmax = 0;
                switch (ent->m_Type)
                {
                case ENTITY_PLAYER:
                    health    = CE_INT(ent, netvar.iHealth);
                    healthmax = ent->m_iMaxHealth;
                    break;
                case ENTITY_BUILDING:
                    health    = CE_INT(ent, netvar.iBuildingHealth);
                    healthmax = CE_INT(ent, netvar.iBuildingMaxHealth);
                    break;
                }

                // Get Colors
                rgba_t hp = colors::Transparent(
                    colors::Health(health, healthmax), fg.a);
                rgba_t border =
                    ((ent->m_iClassID == RCC_PLAYER) && IsPlayerInvisible(ent))
                        ? colors::FromRGBA8(160, 160, 160, fg.a * 255.0f)
                        : colors::Transparent(colors::black, fg.a);
                // Get bar height
                int hbh = (max_y - min_y - 2) *
                          std::min((float) health / (float) healthmax, 1.0f);

                // Draw
                draw_api::draw_rect_outlined(min_x - 7, min_y, 7, max_y - min_y,
                                             border, 0.5f);
                draw_api::draw_rect(min_x - 6, max_y - hbh - 1, 5, hbh, hp);
            }
        }
    }

    // Check if entity has strings to draw
    if (ent_data.string_count)
    {
        PROF_SECTION(PT_esp_drawstrings);

        // Create our initial point at the center of the entity
        Vector draw_point   = screen;
        bool origin_is_zero = true;

        // Only get collidable for players and buildings
        if (ent->m_Type == ENTITY_PLAYER || ent->m_Type == ENTITY_BUILDING)
        {

            // Get collidable from the cache
            if (GetCollide(ent))
            {

                // Origin could change so we set to false
                origin_is_zero = false;

                // Pull the cached collide info
                int max_x = ent_data.collide_max.x;
                int max_y = ent_data.collide_max.y;
                int min_x = ent_data.collide_min.x;
                int min_y = ent_data.collide_min.y;

                // Change the position of the draw point depending on the user
                // settings
                switch ((int) esp_text_position)
                {
                case 0:
                { // TOP RIGHT
                    draw_point = Vector(max_x + 2, min_y, 0);
                }
                break;
                case 1:
                { // BOTTOM RIGHT
                    draw_point =
                        Vector(max_x + 2,
                               max_y -
                                   data.at(ent->m_IDX).string_count *
                                       /*((int)fonts::font_main->height)*/ 14,
                               0);
                }
                break;
                case 2:
                { // CENTER
                    origin_is_zero =
                        true; // origin is still zero so we set to true
                }
                break;
                case 3:
                { // ABOVE
                    draw_point =
                        Vector(min_x,
                               min_y -
                                   data.at(ent->m_IDX).string_count *
                                       /*((int)fonts::font_main->height)*/ 14,
                               0);
                }
                break;
                case 4:
                { // BELOW
                    draw_point = Vector(min_x, max_y, 0);
                }
                }
            }
        }

        // if user setting allows vis check and ent isnt visable, make
        // transparent
        if (vischeck && !ent->IsVisible())
            transparent = true;

        // Loop through strings
        for (int j = 0; j < ent_data.string_count; j++)
        {

            // Pull string from the entity's cached string array
            const ESPString &string = ent_data.strings[j];

            // If string has a color assined to it, apply that otherwise use
            // entities color
            rgba_t color = string.color ? string.color : ent_data.color;
            if (transparent)
                color =
                    colors::Transparent(color); // Apply transparency if needed

            // If the origin is centered, we use one method. if not, the other
            if (!origin_is_zero || true)
            {
                draw_api::draw_string_with_outline(
                    draw_point.x, draw_point.y, string.data.c_str(),
                    fonts::main_font, color, colors::black, 1.5f);
            }
            else
            { /*
          int size_x;
          FTGL_StringLength(string.data, fonts::font_main, &size_x);
          FTGL_Draw(string.data, draw_point.x - size_x / 2, draw_point.y,
          fonts::font_main, color);
      */
            }

            // Add to the y due to their being text in that spot
            draw_point.y += /*((int)fonts::font_main->height)*/ 15 - 1;
        }
    }

    // TODO Add Rotation matix
    // TODO Currently crashes, needs null check somewhere
    // Draw Hitboxes
    /*if (draw_hitbox && ent->m_Type == ENTITY_PLAYER) {
        PROF_SECTION(PT_esp_drawhitbboxes);

        // Loop through hitboxes
        for (int i = 0; i <= 17; i++) { // I should probs get how many hitboxes
    instead of using a fixed number...

            // Get a hitbox from the entity
            hitbox_cache::CachedHitbox* hb = ent->hitboxes.GetHitbox(i);

            // Create more points from min + max
            Vector box_points[8];
            Vector vec_tmp;
            for (int ii = 0; ii <= 8; ii++) { // 8 points to the box

                // logic le paste from sdk
                vec_tmp[0] = ( ii & 0x1 ) ? hb->max[0] : hb->min[0];
                vec_tmp[1] = ( ii & 0x2 ) ? hb->max[1] : hb->min[1];
                vec_tmp[2] = ( ii & 0x4 ) ? hb->max[2] : hb->min[2];

                // save to points array
                box_points[ii] = vec_tmp;
            }

            // Draw box from points
            // Draws a point to every other point. Ineffient, use now fix
    later... Vector scn1, scn2; // to screen for (int ii = 0; ii < 8; ii++) {

                // Get first point
                if (!draw::WorldToScreen(box_points[ii], scn1)) continue;

                for (int iii = 0; iii < 8; iii++) {

                    // Get second point
                    if (!draw::WorldToScreen(box_points[iii], scn2)) continue;

                    // Draw between points
                    draw_api::Line(scn1.x, scn1.y, scn2.x - scn1.x, scn2.y -
    scn1.y, fg);
                }
            }
        }
    }*/
}

// Used to process entities from CreateMove
void _FASTCALL ProcessEntity(CachedEntity *ent)
{
    if (!enabled)
        return; // Esp enable check
    if (CE_BAD(ent))
        return; // CE_BAD check to prevent crashes

    // Entity esp
    if (entity_info)
    {
        AddEntityString(ent,
                        format(RAW_ENT(ent)->GetClientClass()->m_pNetworkName,
                               " [", ent->m_iClassID, "]"));
        if (entity_id)
        {
            AddEntityString(ent, std::to_string(ent->m_IDX));
        }
        if (entity_model)
        {
            const model_t *model = RAW_ENT(ent)->GetModel();
            if (model)
                AddEntityString(ent,
                                std::string(g_IModelInfo->GetModelName(model)));
        }
    }

    // Get esp data from current ent
    ESPData &espdata = data[ent->m_IDX];

    // Projectile esp
    if (ent->m_Type == ENTITY_PROJECTILE && proj_esp &&
        (ent->m_bEnemy || (teammates && !proj_enemy)))
    {

        // Rockets
        if (ent->m_iClassID == CL_CLASS(CTFProjectile_Rocket) ||
            ent->m_iClassID == CL_CLASS(CTFProjectile_SentryRocket))
        {
            if (proj_rockets)
            {
                if ((int) proj_rockets != 2 || ent->m_bCritProjectile)
                {
                    AddEntityString(ent, "[ ==> ]");
                }
            }

            // Pills/Stickys
        }
        else if (ent->m_iClassID == CL_CLASS(CTFGrenadePipebombProjectile))
        {
            // Switch based on pills/stickys
            switch (CE_INT(ent, netvar.iPipeType))
            {
            case 0: // Pills
                if (!proj_pipes)
                    break;
                if ((int) proj_pipes == 2 && !ent->m_bCritProjectile)
                    break;
                AddEntityString(ent, "[ (PP) ]");
                break;
            case 1: // Stickys
                if (!proj_stickies)
                    break;
                if ((int) proj_stickies == 2 && !ent->m_bCritProjectile)
                    break;
                AddEntityString(ent, "[ {*} ]");
            }

            // Huntsman
        }
        else if (ent->m_iClassID == CL_CLASS(CTFProjectile_Arrow))
        {
            if ((int) proj_arrows != 2 || ent->m_bCritProjectile)
            {
                AddEntityString(ent, "[ >>---> ]");
            }
        }
    }

    // Hl2DM dropped item esp
    IF_GAME(IsHL2DM())
    {
        if (item_esp && item_dropped_weapons)
        {
            if (CE_BYTE(ent, netvar.hOwner) == (unsigned char) -1)
            {
                int string_count_backup = data[ent->m_IDX].string_count;
                if (ent->m_iClassID == CL_CLASS(CWeapon_SLAM))
                    AddEntityString(ent, "SLAM");
                else if (ent->m_iClassID == CL_CLASS(CWeapon357))
                    AddEntityString(ent, ".357");
                else if (ent->m_iClassID == CL_CLASS(CWeaponAR2))
                    AddEntityString(ent, "AR2");
                else if (ent->m_iClassID == CL_CLASS(CWeaponAlyxGun))
                    AddEntityString(ent, "Alyx Gun");
                else if (ent->m_iClassID == CL_CLASS(CWeaponAnnabelle))
                    AddEntityString(ent, "Annabelle");
                else if (ent->m_iClassID == CL_CLASS(CWeaponBinoculars))
                    AddEntityString(ent, "Binoculars");
                else if (ent->m_iClassID == CL_CLASS(CWeaponBugBait))
                    AddEntityString(ent, "Bug Bait");
                else if (ent->m_iClassID == CL_CLASS(CWeaponCrossbow))
                    AddEntityString(ent, "Crossbow");
                else if (ent->m_iClassID == CL_CLASS(CWeaponShotgun))
                    AddEntityString(ent, "Shotgun");
                else if (ent->m_iClassID == CL_CLASS(CWeaponSMG1))
                    AddEntityString(ent, "SMG");
                else if (ent->m_iClassID == CL_CLASS(CWeaponRPG))
                    AddEntityString(ent, "RPG");
                if (string_count_backup != data[ent->m_IDX].string_count)
                {
                    SetEntityColor(ent, colors::yellow);
                }
            }
        }
    }

    // Tank esp
    if (ent->m_iClassID == CL_CLASS(CTFTankBoss) && tank)
    {
        AddEntityString(ent, "Tank");

        // Dropped weapon esp
    }
    else if (ent->m_iClassID == CL_CLASS(CTFDroppedWeapon) && item_esp &&
             item_dropped_weapons)
    {
        AddEntityString(
            ent, format("WEAPON ", RAW_ENT(ent)->GetClientClass()->GetName()));

        // MVM Money esp
    }
    else if (ent->m_iClassID == CL_CLASS(CCurrencyPack) && item_money)
    {
        if (CE_BYTE(ent, netvar.bDistributed))
        {
            if (item_money_red)
            {
                AddEntityString(ent, "~$~");
            }
        }
        else
        {
            AddEntityString(ent, "$$$");
        }

        // Other item esp
    }
    else if (ent->m_ItemType != ITEM_NONE && item_esp)
    {

        // Health pack esp
        if (item_health_packs && (ent->m_ItemType >= ITEM_HEALTH_SMALL &&
                                      ent->m_ItemType <= ITEM_HEALTH_LARGE ||
                                  ent->m_ItemType == ITEM_HL_BATTERY))
        {
            if (ent->m_ItemType == ITEM_HEALTH_SMALL)
                AddEntityString(ent, "[+]");
            if (ent->m_ItemType == ITEM_HEALTH_MEDIUM)
                AddEntityString(ent, "[++]");
            if (ent->m_ItemType == ITEM_HEALTH_LARGE)
                AddEntityString(ent, "[+++]");
            if (ent->m_ItemType == ITEM_HL_BATTERY)
                AddEntityString(ent, "[Z]");

            // TF2C Adrenaline esp
        }
        else if (item_adrenaline && ent->m_ItemType == ITEM_TF2C_PILL)
        {
            AddEntityString(ent, "[a]");

            // Ammo pack esp
        }
        else if (item_ammo_packs && ent->m_ItemType >= ITEM_AMMO_SMALL &&
                 ent->m_ItemType <= ITEM_AMMO_LARGE)
        {
            if (ent->m_ItemType == ITEM_AMMO_SMALL)
                AddEntityString(ent, "{i}");
            if (ent->m_ItemType == ITEM_AMMO_MEDIUM)
                AddEntityString(ent, "{ii}");
            if (ent->m_ItemType == ITEM_AMMO_LARGE)
                AddEntityString(ent, "{iii}");

            // Powerup esp
        }
        else if (item_powerups && ent->m_ItemType >= ITEM_POWERUP_FIRST &&
                 ent->m_ItemType <= ITEM_POWERUP_LAST)
        {
            AddEntityString(
                ent, format(powerups[ent->m_ItemType - ITEM_POWERUP_FIRST],
                            " PICKUP"));

            // TF2C weapon spawner esp
        }
        else if (item_weapon_spawners && ent->m_ItemType >= ITEM_TF2C_W_FIRST &&
                 ent->m_ItemType <= ITEM_TF2C_W_LAST)
        {
            AddEntityString(
                ent,
                format(tf2c_weapon_names[ent->m_ItemType - ITEM_TF2C_W_FIRST],
                       " SPAWNER"));
            if (CE_BYTE(ent, netvar.bRespawning))
                AddEntityString(ent, "-- RESPAWNING --");

            // Halloween spell esp
        }
        else if (item_spellbooks && (ent->m_ItemType == ITEM_SPELL ||
                                     ent->m_ItemType == ITEM_SPELL_RARE))
        {
            if (ent->m_ItemType == ITEM_SPELL)
            {
                AddEntityString(ent, "Spell", colors::green);
            }
            else
            {
                AddEntityString(ent, "Rare Spell",
                                colors::FromRGBA8(139, 31, 221, 255));
            }
        }

        // Building esp
    }
    else if (ent->m_Type == ENTITY_BUILDING && buildings)
    {

        // Check if enemy building
        if (!ent->m_bEnemy && !teammates)
            return;

        // TODO maybe...
        /*if (legit && ent->m_iTeam != g_pLocalPlayer->team) {
            if (ent->m_lLastSeen > v_iLegitSeenTicks->GetInt()) {
                return;
            }
        }*/

        // Make a name for the building based on the building type and level
        if (show_name || show_class)
        {
            const std::string &name =
                (ent->m_iClassID == CL_CLASS(CObjectTeleporter)
                     ? "Teleporter"
                     : (ent->m_iClassID == CL_CLASS(CObjectSentrygun)
                            ? "Sentry Gun"
                            : "Dispenser"));
            int level = CE_INT(ent, netvar.iUpgradeLevel);
            AddEntityString(ent, format("LV ", level, ' ', name));
        }
        // If text health is true, then add a string with the health
        if ((int) show_health == 1 || (int) show_health == 3)
        {
            AddEntityString(
                ent, format(ent->m_iHealth, '/', ent->m_iMaxHealth, " HP"),
                colors::Health(ent->m_iHealth, ent->m_iMaxHealth));
        }
        // Set the entity to repaint
        espdata.needs_paint = true;
        return;

        // Player esp
    }
    else if (ent->m_Type == ENTITY_PLAYER && ent->m_bAlivePlayer)
    {

        // Local player handling
        if (!(local_esp && g_IInput->CAM_IsThirdPerson()) &&
            ent->m_IDX == g_IEngine->GetLocalPlayer())
            return;

        // Get player class
        int pclass = CE_INT(ent, netvar.iClass);

        // Attempt to get player info, and if cant, return
        player_info_s info;
        if (!g_IEngine->GetPlayerInfo(ent->m_IDX, &info))
            return;

        // TODO, check if u can just use "ent->m_bEnemy" instead of m_iTeam
        // Legit mode handling
        if (legit && ent->m_iTeam != g_pLocalPlayer->team &&
            playerlist::IsDefault(info.friendsID))
        {
            if (IsPlayerInvisible(ent))
                return; // Invis check
            if (vischeck && !ent->IsVisible())
                return; // Vis check
                        // TODO, maybe...
                        // if (ent->m_lLastSeen >
                        // (unsigned)v_iLegitSeenTicks->GetInt())
            // return;
        }

        // Powerup handling
        if (powerup_esp)
        {
            powerup_type power = GetPowerupOnPlayer(ent);
            if (power != not_powerup)
                AddEntityString(ent, format("^ ", powerups[power], " ^"));
        }

        // Dont understand reasoning for this check
        if (ent->m_bEnemy || teammates ||
            !playerlist::IsDefault(info.friendsID))
        {

            // Playername
            if (show_name)
                AddEntityString(ent, std::string(info.name));

            // Player class
            if (show_class)
            {
                if (pclass > 0 && pclass < 10)
                    AddEntityString(ent, classes[pclass - 1]);
            }

#if ENABLE_IPC == 1
            // ipc bot esp
            if (show_bot_id && ipc::peer && ent != LOCAL_E)
            {
                for (unsigned i = 0; i < cat_ipc::max_peers; i++)
                {
                    if (!ipc::peer->memory->peer_data[i].free &&
                        ipc::peer->memory->peer_user_data[i].friendid ==
                            info.friendsID)
                    {
                        AddEntityString(ent, format("BOT #", i));
                        break;
                    }
                }
            }
#endif
            // Health esp
            if ((int) show_health == 1 || (int) show_health == 3)
            {
                AddEntityString(
                    ent, format(ent->m_iHealth, '/', ent->m_iMaxHealth, " HP"),
                    colors::Health(ent->m_iHealth, ent->m_iMaxHealth));
            }
            IF_GAME(IsTF())
            {
                // Medigun Ubercharge esp
                if (show_ubercharge)
                {
                    if (CE_INT(ent, netvar.iClass) == tf_medic)
                    {
                        int *weapon_list = (int *) ((unsigned) (RAW_ENT(ent)) +
                                                    netvar.hMyWeapons);
                        for (int i = 0; weapon_list[i]; i++)
                        {
                            int handle = weapon_list[i];
                            int eid    = handle & 0xFFF;
                            if (eid >= 32 && eid <= HIGHEST_ENTITY)
                            {
                                CachedEntity *weapon = ENTITY(eid);
                                if (!CE_BAD(weapon) &&
                                    weapon->m_iClassID ==
                                        CL_CLASS(CWeaponMedigun) &&
                                    weapon)
                                {
                                    if (CE_INT(weapon,
                                               netvar.iItemDefinitionIndex) !=
                                        998)
                                    {
                                        AddEntityString(
                                            ent,
                                            format(
                                                floor(
                                                    CE_FLOAT(
                                                        weapon,
                                                        netvar
                                                            .m_flChargeLevel) *
                                                    100),
                                                '%', " Uber"),
                                            colors::Health(
                                                (CE_FLOAT(
                                                     weapon,
                                                     netvar.m_flChargeLevel) *
                                                 100),
                                                100));
                                    }
                                    else
                                        AddEntityString(
                                            ent,
                                            format(
                                                floor(
                                                    CE_FLOAT(
                                                        weapon,
                                                        netvar
                                                            .m_flChargeLevel) *
                                                    100),
                                                '%', " Uber | Charges: ",
                                                floor(
                                                    CE_FLOAT(
                                                        weapon,
                                                        netvar
                                                            .m_flChargeLevel) /
                                                    0.25f)),
                                            colors::Health(
                                                (CE_FLOAT(
                                                     weapon,
                                                     netvar.m_flChargeLevel) *
                                                 100),
                                                100));
                                    break;
                                }
                            }
                        }
                    }
                }
                // Conditions esp
                if (show_conditions)
                {
                    const auto &clr = colors::EntityF(ent);
                    // Invis
                    if (IsPlayerInvisible(ent))
                        AddEntityString(
                            ent, "*CLOAKED*",
                            colors::FromRGBA8(220.0f, 220.0f, 220.0f, 255.0f));
                    // Uber/Bonk
                    if (IsPlayerInvulnerable(ent))
                        AddEntityString(ent, "*INVULNERABLE*");
                    // Vaccinator
                    if (HasCondition<TFCond_UberBulletResist>(ent))
                    {
                        AddEntityString(ent, "*VACCINATOR*");
                    }
                    else if (HasCondition<TFCond_SmallBulletResist>(ent))
                    {
                        AddEntityString(ent, "*PASSIVE RESIST*");
                    }
                    // Crit
                    if (IsPlayerCritBoosted(ent))
                        AddEntityString(ent, "*CRITS*", colors::orange);
                    // Zoomed
                    if (HasCondition<TFCond_Zoomed>(ent))
                    {
                        AddEntityString(
                            ent, "*ZOOMING*",
                            colors::FromRGBA8(220.0f, 220.0f, 220.0f, 255.0f));
                        // Slowed
                    }
                    else if (HasCondition<TFCond_Slowed>(ent))
                    {
                        AddEntityString(
                            ent, "*SLOWED*",
                            colors::FromRGBA8(220.0f, 220.0f, 220.0f, 255.0f));
                    }
                    // Jarated
                    if (HasCondition<TFCond_Jarated>(ent))
                        AddEntityString(ent, "*JARATED*", colors::yellow);
                }
            }
            // Hoovy Esp
            if (IsHoovy(ent))
                AddEntityString(ent, "Hoovy");

            // Active weapon esp
            int widx = CE_INT(ent, netvar.hActiveWeapon) & 0xFFF;
            if (IDX_GOOD(widx))
            {
                CachedEntity *weapon = ENTITY(widx);
                if (CE_GOOD(weapon))
                {
                    if (show_weapon)
                    {
                        const char *weapon_name =
                            re::C_BaseCombatWeapon::GetPrintName(
                                RAW_ENT(weapon));
                        if (weapon_name)
                            AddEntityString(ent, std::string(weapon_name));
                    }
                }
            }

            // Notify esp to repaint
            espdata.needs_paint = true;
        }
        return;
    }
}

// Draw a box around a player
void _FASTCALL DrawBox(CachedEntity *ent, const rgba_t &clr)
{
    PROF_SECTION(PT_esp_drawbox);

    // Check if ent is bad to prevent crashes
    if (CE_BAD(ent))
        return;

    // Get our collidable bounds
    if (!GetCollide(ent))
        return;

    // Pull the cached collide info
    ESPData &ent_data = data[ent->m_IDX];
    int max_x         = ent_data.collide_max.x;
    int max_y         = ent_data.collide_max.y;
    int min_x         = ent_data.collide_min.x;
    int min_y         = ent_data.collide_min.y;

    // Depending on whether the player is cloaked, we change the color
    // acordingly
    rgba_t border = ((ent->m_iClassID == RCC_PLAYER) && IsPlayerInvisible(ent))
                        ? colors::FromRGBA8(160, 160, 160, clr.a * 255.0f)
                        : colors::Transparent(colors::black, clr.a);

    // With box corners, we draw differently
    if ((int) box_esp == 2)
        BoxCorners(min_x, min_y, max_x, max_y, clr, (clr.a != 1.0f));
    // Otherwise, we just do simple draw funcs
    else
    {
        draw_api::draw_rect_outlined(min_x, min_y, max_x - min_x, max_y - min_y,
                                     border, 0.5f);
        draw_api::draw_rect_outlined(min_x + 1, min_y + 1, max_x - min_x - 2,
                                     max_y - min_y - 2, clr, 0.5f);
        draw_api::draw_rect_outlined(min_x + 2, min_y + 2, max_x - min_x - 4,
                                     max_y - min_y - 4, border, 0.5f);
    }
}

// Function to draw box corners, Used by DrawBox
void BoxCorners(int minx, int miny, int maxx, int maxy, const rgba_t &color,
                bool transparent)
{
    const rgba_t &black =
        transparent ? colors::Transparent(colors::black) : colors::black;
    const int size = box_corner_size;

    // Black corners
    // Top Left
    draw_api::draw_rect(minx, miny, size, 3, black);
    draw_api::draw_rect(minx, miny + 3, 3, size - 3, black);
    // Top Right
    draw_api::draw_rect(maxx - size + 1, miny, size, 3, black);
    draw_api::draw_rect(maxx - 3 + 1, miny + 3, 3, size - 3, black);
    // Bottom Left
    draw_api::draw_rect(minx, maxy - 3, size, 3, black);
    draw_api::draw_rect(minx, maxy - size, 3, size - 3, black);
    // Bottom Right
    draw_api::draw_rect(maxx - size + 1, maxy - 3, size, 3, black);
    draw_api::draw_rect(maxx - 2, maxy - size, 3, size - 3, black);

    // Colored corners
    // Top Left
    draw_api::draw_line(minx + 1, miny + 1, size - 2, 0, color, 0.5f);
    draw_api::draw_line(minx + 1, miny + 1, 0, size - 2, color, 0.5f);
    // Top Right
    draw_api::draw_line(maxx - 1, miny + 1, -(size - 2), 0, color, 0.5f);
    draw_api::draw_line(maxx - 1, miny + 1, 0, size - 2, color, 0.5f);
    // Bottom Left
    draw_api::draw_line(minx + 1, maxy - 2, size - 2, 0, color, 0.5f);
    draw_api::draw_line(minx + 1, maxy - 2, 0, -(size - 2), color, 0.5f);
    // Bottom Right
    draw_api::draw_line(maxx - 1, maxy - 2, -(size - 2), 0, color, 0.5f);
    draw_api::draw_line(maxx - 1, maxy - 2, 0, -(size - 2), color, 0.5f);
}

// Used for caching collidable bounds
bool GetCollide(CachedEntity *ent)
{
    PROF_SECTION(PT_esp_getcollide);

    // Null + Dormant check to prevent crashing
    if (CE_BAD(ent))
        return false;

    // Grab esp data
    ESPData &ent_data = data[ent->m_IDX];

    // If entity has cached collides, return it. Otherwise generate new bounds
    if (!ent_data.has_collide)
    {

        // Get collision center, max, and mins
        const Vector &origin =
            RAW_ENT(ent)->GetCollideable()->GetCollisionOrigin();
        Vector mins = RAW_ENT(ent)->GetCollideable()->OBBMins() + origin;
        Vector maxs = RAW_ENT(ent)->GetCollideable()->OBBMaxs() + origin;

        // Create a array for storing box points
        Vector points_r[8]; // World vectors
        Vector points[8];   // Screen vectors

        // If user setting for box expnad is true, spread the max and mins
        if (esp_expand)
        {
            const float &exp = (float) esp_expand;
            maxs.x += exp;
            maxs.y += exp;
            maxs.z += exp;
            mins.x -= exp;
            mins.y -= exp;
            mins.z -= exp;
        }

        // Create points for the box based on max and mins
        float x     = maxs.x - mins.x;
        float y     = maxs.y - mins.y;
        float z     = maxs.z - mins.z;
        points_r[0] = mins;
        points_r[1] = mins + Vector(x, 0, 0);
        points_r[2] = mins + Vector(x, y, 0);
        points_r[3] = mins + Vector(0, y, 0);
        points_r[4] = mins + Vector(0, 0, z);
        points_r[5] = mins + Vector(x, 0, z);
        points_r[6] = mins + Vector(x, y, z);
        points_r[7] = mins + Vector(0, y, z);

        // Check if any point of the box isnt on the screen
        bool success = true;
        for (int i = 0; i < 8; i++)
        {
            if (!draw::WorldToScreen(points_r[i], points[i]))
                success = false;
        }
        // If a point isnt on the screen, return here
        if (!success)
            return false;

        // Get max and min of the box using the newly created screen vector
        int max_x = -1;
        int max_y = -1;
        int min_x = 65536;
        int min_y = 65536;
        for (int i = 0; i < 8; i++)
        {
            if (points[i].x > max_x)
                max_x = points[i].x;
            if (points[i].y > max_y)
                max_y = points[i].y;
            if (points[i].x < min_x)
                min_x = points[i].x;
            if (points[i].y < min_y)
                min_y = points[i].y;
        }

        // Save the info to the esp data and notify cached that we cached info.
        ent_data.collide_max = Vector(max_x, max_y, 0);
        ent_data.collide_min = Vector(min_x, min_y, 0);
        ent_data.has_collide = true;

        return true;
    }
    else
    {
        // We already have collidable so return true.
        return true;
    }
    // Impossible error, return false
    return false;
}

// Use to add a esp string to an entity
void AddEntityString(CachedEntity *entity, const std::string &string,
                     const rgba_t &color)
{
    ESPData &entity_data = data[entity->m_IDX];
    if (entity_data.string_count >= 15)
        return;
    entity_data.strings[entity_data.string_count].data  = string;
    entity_data.strings[entity_data.string_count].color = color;
    entity_data.string_count++;
    entity_data.needs_paint = true;
}

// Function to reset entitys strings
void ResetEntityStrings()
{
    for (auto &i : data)
    {
        i.string_count = 0;
        i.color        = colors::empty;
        i.needs_paint  = false;
    }
}

// Sets an entitys esp color
void SetEntityColor(CachedEntity *entity, const rgba_t &color)
{
    data[entity->m_IDX].color = color;
}
}
}
}
