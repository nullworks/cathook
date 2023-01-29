/*
 * Misc.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <unistd.h>
#include <regex>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <hacks/AntiAim.hpp>
#include <settings/Bool.hpp>

#include "core/sharedobj.hpp"
#include "filesystem.h"
#include "DetourHook.hpp"
#include "AntiCheatBypass.hpp"
#include <Warp.hpp>
#include "hack.hpp"
#include <thread>
namespace hacks::shared::misc
{
#if !ENFORCE_STREAM_SAFETY && ENABLE_VISUALS
static settings::Boolean render_zoomed{ "visual.render-local-zoomed", "true" };
#endif
static settings::Boolean anti_afk{ "misc.anti-afk", "false" };
static settings::Int auto_strafe{ "misc.autostrafe", "0" };
static settings::Boolean tauntslide{ "misc.tauntslide-tf2c", "false" };
static settings::Boolean tauntslide_tf2{ "misc.tauntslide", "false" };
static settings::Boolean flashlight_spam{ "misc.flashlight-spam", "false" };
static settings::Boolean auto_balance_spam{ "misc.auto-balance-spam", "false" };
static settings::Boolean nopush_enabled{ "misc.no-push", "false" };
static settings::Boolean dont_hide_stealth_kills{ "misc.dont-hide-stealth-kills", "true" };
static settings::Boolean unlimit_bumpercart_movement{ "misc.bumpercarthax.enable", "true" };
static settings::Boolean ping_reducer{ "misc.ping-reducer.enable", "false" };
static settings::Int force_ping{ "misc.ping-reducer.target", "0" };
static settings::Boolean force_wait{ "misc.force-enable-wait", "true" };
static settings::Boolean scc{ "misc.scoreboard.match-custom-team-colors", "false" };
#if ENABLE_VISUALS
static settings::Boolean debug_info{ "misc.debug-info", "false" };
static settings::Boolean show_spectators{ "misc.show-spectators", "false" };
static settings::Boolean misc_drawhitboxes{ "misc.draw-hitboxes", "false" };
// Useful for debugging with showlagcompensation
static settings::Boolean misc_drawhitboxes_dead{ "misc.draw-hitboxes.dead-players", "false" };
#endif

#if ENABLE_TEXTMODE
static settings::Boolean fix_cyoaanim{ "remove.contracker", "true" };
#else
static settings::Boolean fix_cyoaanim{ "remove.contracker", "false" };
#endif

#if !ENFORCE_STREAM_SAFETY && ENABLE_VISUALS
static void tryPatchLocalPlayerShouldDraw(bool after)
{
    static BytePatch patch_shoulddraw{ gSignatures.GetClientSignature, "80 BB ? ? ? ? ? 75 DE", 0xD, { 0xE0 } };
    static BytePatch patch_shoulddraw_wearable{ gSignatures.GetClientSignature("0F 85 ? ? ? ? E9 ? ? ? ? 85 D2") + 1, { 0x80 } };

    if (after)
    {
        patch_shoulddraw.Patch();
        patch_shoulddraw_wearable.Patch();
    }
    else
    {
        patch_shoulddraw.Shutdown();
        patch_shoulddraw_wearable.Shutdown();
    }
}
#endif

static Timer anti_afk_timer{};
static int last_buttons{ 0 };

static void updateAntiAfk()
{
    if (current_user_cmd->buttons != last_buttons || g_pLocalPlayer->life_state)
    {
        anti_afk_timer.update();
        last_buttons = current_user_cmd->buttons;
    }
    else
    {
        static auto afk_timer = g_ICvar->FindVar("mp_idlemaxtime");
        if (!afk_timer)
            afk_timer = g_ICvar->FindVar("mp_idlemaxtime");
        // Trigger 10 seconds before kick
        else if (afk_timer->GetInt() != 0 && anti_afk_timer.check(afk_timer->m_nValue * 60 * 1000 - 10000))
        {
            // Game also checks if you move if you are in spawn, so spam movement keys alternatingly
            bool flip = false;
            current_user_cmd->buttons |= flip ? IN_FORWARD : IN_BACK;
            // Flip flip
            flip = !flip;
            if (anti_afk_timer.check(afk_timer->GetInt() * 60 * 1000 + 1000))
            {
                anti_afk_timer.update();
            }
        }
        last_buttons = current_user_cmd->buttons;
    }
}

CatCommand fix_cursor("fix_cursor", "Fix the GUI cursor being visible",
                      []()
                      {
                          g_ISurface->LockCursor();
                          g_ISurface->SetCursorAlwaysVisible(false);
                      });

// Use to send a autobalance request to the server that doesnt prevent you from
// using it again, Allowing infinite use of it.
void SendAutoBalanceRequest()
{ // Credits to blackfire
    if (!g_IEngine->IsInGame())
        return;
    KeyValues *kv = new KeyValues("AutoBalanceVolunteerReply");
    kv->SetInt("response", 1);
    g_IEngine->ServerCmdKeyValues(kv);
}

// Catcommand for above
CatCommand SendAutoBlRqCatCom("request_balance", "Request Infinite Auto-Balance", [](const CCommand &args) { SendAutoBalanceRequest(); });

int last_number{ 0 };
static bool flash_light_spam_switch{ false };
static Timer auto_balance_timer{};

static ConVar *teammatesPushaway{ nullptr };

int getCarriedBuilding()
{
    if (CE_INT(LOCAL_E, netvar.m_bCarryingObject))
        return HandleToIDX(CE_INT(LOCAL_E, netvar.m_hCarriedObject));
    for (auto const &ent : entity_cache::valid_ents)
    {
        if (ent->m_Type() != ENTITY_BUILDING)
            continue;
        if (HandleToIDX(CE_INT(ent, netvar.m_hBuilder)) != LOCAL_E->m_IDX)
            continue;
        if (!CE_INT(ent, netvar.m_bPlacing))
            continue;
        return ent->m_IDX;
    }
    return -1;
}
#if ENABLE_VISUALS

struct wireframe_data
{
    Vector raw_min;
    Vector raw_max;
    Vector rotation;
    Vector origin;
};

std::vector<wireframe_data> wireframe_queue;
void QueueWireframeHitboxes(hitbox_cache::EntityHitboxCache &hb_cache)
{
    for (int i = 0; i < hb_cache.GetNumHitboxes(); ++i)
    {
        auto hb        = hb_cache.GetHitbox(i);
        Vector raw_min = hb->bbox->bbmin;
        Vector raw_max = hb->bbox->bbmax;
        auto transform = hb_cache.GetBones()[hb->bbox->bone];
        Vector rotation;
        Vector origin;

        MatrixAngles(transform, *(QAngle *) &rotation, origin);
        wireframe_queue.push_back(wireframe_data{ raw_min, raw_max, rotation, origin });
    }
}
void DrawWireframeHitbox(wireframe_data data)
{
    g_IVDebugOverlay->AddBoxOverlay2(data.origin, data.raw_min, data.raw_max, VectorToQAngle(data.rotation), Color(0, 0, 0, 0), Color(255, 0, 0, 255), g_GlobalVars->interval_per_tick * 2);
}

#endif

static float normalizeRad(float a) noexcept
{
    return std::isfinite(a) ? std::remainder(a, PI * 2) : 0.0f;
}
static float angleDiffRad(float a1, float a2) noexcept
{
    float delta;

    delta = normalizeRad(a1 - a2);
    if (a1 > a2)
    {
        if (delta >= PI)
            delta -= PI * 2;
    }
    else
    {
        if (delta <= -PI)
            delta += PI * 2;
    }
    return delta;
}

static void CreateMove()
{
#if ENABLE_VISUALS
    if (misc_drawhitboxes)
    {
        for (auto const &ent : entity_cache::player_cache)
        {

            if (CE_INVALID(ent) || ent == LOCAL_E || (!misc_drawhitboxes_dead && !ent->m_bAlivePlayer()))
                continue;
            QueueWireframeHitboxes(ent->hitboxes);
        }
    }
#endif
    if (current_user_cmd->command_number)
        last_number = current_user_cmd->command_number;
    // AntiAfk That after a certian time without movement keys depressed, causes
    // random keys to be spammed for 1 second
    if (anti_afk)
        updateAntiAfk();

    // Automatically strafes in the air
    if (auto_strafe && CE_GOOD(LOCAL_E) && !g_pLocalPlayer->life_state)
    {
        auto flags    = CE_INT(LOCAL_E, netvar.iFlags);
        auto movetype = (unsigned) CE_VAR(LOCAL_E, 0x194, unsigned char);

        // Noclip
        if (movetype != 8)
        {
            switch (*auto_strafe)
            {
            case 0: // Off
                break;
            case 1: // Regular strafe
            {
                static bool was_jumping = false;
                bool is_jumping         = current_user_cmd->buttons & IN_JUMP;

                if (!(flags & FL_ONGROUND) && !(flags & FL_INWATER) && (!is_jumping || was_jumping))
                    if (current_user_cmd->mousedx)
                        current_user_cmd->sidemove = current_user_cmd->mousedx > 1 ? 450.f : -450.f;

                was_jumping = is_jumping;

                break;
            }
            case 2: // Multidirectional Airstrafe,
                    // Huge Credits to https://github.com/degeneratehyperbola/NEPS, as their airstrafe
                    // Apparently just works for tf2
                    // Also credits to "zelta" for porting it to tf2,
                    // And "Cyanide" for making it work with cathook.
            {
                const float speed = CE_VECTOR(LOCAL_E, netvar.vVelocity).Length2D();
                auto vel          = CE_VECTOR(LOCAL_E, netvar.vVelocity);

                if (flags & FL_ONGROUND || flags & FL_INWATER)
                    break;
                if (speed < 2.0f)
                    break;

                constexpr auto perfectDelta = [](float speed) noexcept
                {
                    static auto speedVar = CE_FLOAT(LOCAL_E, netvar.m_flMaxspeed);
                    static auto airVar   = g_ICvar->FindVar("sv_airaccelerate");
                    // This is hardcoded for tf2, unless you run sourcemod
                    static auto wishSpeed = 30.0f;

                    const auto term = wishSpeed / airVar->GetFloat() / speedVar * 100.0f / speed;

                    if (term < 1.0f && term > -1.0f)
                        return acosf(term);

                    return 0.0f;
                };

                const float pDelta = perfectDelta(speed);
                if (pDelta)
                {
                    const float yaw     = DEG2RAD(g_pLocalPlayer->v_OrigViewangles.y);
                    const float velDir  = atan2f(vel.y, vel.x) - yaw;
                    const float wishAng = atan2f(-current_user_cmd->sidemove, current_user_cmd->forwardmove);
                    const float delta   = angleDiffRad(velDir, wishAng); // Helpers::angleDiffRad(velDir, wishAng);

                    float moveDir = delta < 0.0f ? velDir + pDelta : velDir - pDelta;

                    current_user_cmd->forwardmove = cosf(moveDir) * 450.0f;
                    current_user_cmd->sidemove    = -sinf(moveDir) * 450.0f;
                }
                break;
            }
            default:
                break;
            }
        }
    }

    // TF2c Tauntslide
    IF_GAME(IsTF2C())
    {
        if (tauntslide)
            RemoveCondition<TFCond_Taunting>(LOCAL_E);
    }

    // HL2DM flashlight spam
    IF_GAME(IsHL2DM())
    {
        if (flashlight_spam)
        {
            if (flash_light_spam_switch && !current_user_cmd->impulse)
                current_user_cmd->impulse = 100;
            flash_light_spam_switch = !flash_light_spam_switch;
        }
    }

    IF_GAME(IsTF2())
    {
        // Tauntslide needs improvement for movement but it mostly works
        if (tauntslide_tf2)
        {
            // Check to prevent crashing
            if (CE_GOOD(LOCAL_E))
            {
                if (HasCondition<TFCond_Taunting>(LOCAL_E))
                {
                    // get directions
                    float forward = 0;
                    float side    = 0;
                    if (current_user_cmd->buttons & IN_FORWARD)
                        forward += 450;
                    if (current_user_cmd->buttons & IN_BACK)
                        forward -= 450;
                    if (current_user_cmd->buttons & IN_MOVELEFT)
                        side -= 450;
                    if (current_user_cmd->buttons & IN_MOVERIGHT)
                        side += 450;
                    current_user_cmd->forwardmove = forward;
                    current_user_cmd->sidemove    = side;

                    QAngle camera_angle;
                    g_IEngine->GetViewAngles(camera_angle);

                    // Doesnt work with anti-aim as well as I hoped... I guess
                    // this is as far as I can go with such a simple tauntslide
                    if (!hacks::shared::antiaim::isEnabled())
                        current_user_cmd->viewangles.y = camera_angle[1];
                    g_pLocalPlayer->v_OrigViewangles.y = camera_angle[1];

                    // Use silent since we dont want to prevent the player from
                    // looking around
                    g_pLocalPlayer->bUseSilentAngles = true;
                }
            }
        }

        // Spams infinite autobalance spam function
        if (auto_balance_spam && auto_balance_timer.test_and_set(150))
            SendAutoBalanceRequest();

        // Simple No-Push through cvars
        if (teammatesPushaway)
        {
            if (*nopush_enabled == teammatesPushaway->GetBool())
                teammatesPushaway->SetValue(!nopush_enabled);
        }
        else
            teammatesPushaway = g_ICvar->FindVar("tf_avoidteammates_pushaway");

        // Ping Reducer
        if (ping_reducer && !hacks::tf2::antianticheat::enabled)
        {
            static ConVar *cmdrate = g_ICvar->FindVar("cl_cmdrate");
            if (cmdrate == nullptr)
            {
                cmdrate = g_ICvar->FindVar("cl_cmdrate");
                return;
            }
            int ping = g_pPlayerResource->GetPing(g_IEngine->GetLocalPlayer());
            static Timer updateratetimer{};
            if (updateratetimer.test_and_set(500))
            {
                if (*force_ping <= ping)
                {
                    NET_SetConVar command("cl_cmdrate", "-1");
                    ((INetChannel *) g_IEngine->GetNetChannelInfo())->SendNetMsg(command);
                }
                else if (*force_ping > ping)
                {
                    NET_SetConVar command("cl_cmdrate", std::to_string(cmdrate->GetInt()).c_str());
                    ((INetChannel *) g_IEngine->GetNetChannelInfo())->SendNetMsg(command);
                }
            }
        }
    }
}

#if ENABLE_VISUALS
// Timer ussr{};
void Draw()
{
    if (misc_drawhitboxes)
    {
        for (auto const &entry : wireframe_queue)
            DrawWireframeHitbox(entry);
        wireframe_queue.clear();
    }
    /*if (ussr.test_and_set(207000))
    {
        g_ISurface->PlaySound()
    }*/
    if (show_spectators)
    {
        for (auto const &ent: entity_cache::valid_ents)
        {
            // Assign the for loops tick number to an ent
            
            player_info_s info;
            if (ent != LOCAL_E && ent->m_Type() == ENTITY_PLAYER && HandleToIDX(CE_INT(ent, netvar.hObserverTarget)) == LOCAL_E->m_IDX && CE_INT(ent, netvar.iObserverMode) >= 4 && GetPlayerInfo(ent->m_IDX, &info))
            {
                auto observermode = "N/A";
                rgba_t color      = colors::green;

                switch (CE_INT(ent, netvar.iObserverMode))
                {
                case 4:
                    observermode = "1st Person";
                    color        = colors::red_s;
                    break;
                case 5:
                    observermode = "3rd Person";
                    break;
                case 7:
                    observermode = "Freecam";
                    break;
                }
                AddSideString(format(info.name, " - (", observermode, ")"), color);
            }
        }
    }
    if (!debug_info)
        return;
    auto local = LOCAL_W;
    if (CE_GOOD(local))
    {
        AddSideString(format("Slot: ", re::C_BaseCombatWeapon::GetSlot(RAW_ENT(local))));
        AddSideString(format("Taunt Concept: ", CE_INT(LOCAL_E, netvar.m_iTauntConcept)));
        AddSideString(format("Taunt Index: ", CE_INT(LOCAL_E, netvar.m_iTauntIndex)));
        AddSideString(format("Sequence: ", CE_INT(LOCAL_E, netvar.m_nSequence)));
        AddSideString(format("Velocity: ", LOCAL_E->m_vecVelocity.x, ' ', LOCAL_E->m_vecVelocity.y, ' ', LOCAL_E->m_vecVelocity.z));
        AddSideString(format("Velocity3: ", LOCAL_E->m_vecVelocity.Length()));
        AddSideString(format("Velocity2: ", LOCAL_E->m_vecVelocity.Length2D()));
        AddSideString("NetVar Velocity");
        Vector vel = CE_VECTOR(LOCAL_E, netvar.vVelocity);
        AddSideString(format("Velocity: ", vel.x, ' ', vel.y, ' ', vel.z));
        AddSideString(format("Velocity3: ", vel.Length()));
        AddSideString(format("Velocity2: ", vel.Length2D()));
        AddSideString(format("flSimTime: ", LOCAL_E->var<float>(netvar.m_flSimulationTime)));
        if (current_user_cmd)
            AddSideString(format("command_number: ", last_cmd_number));
        AddSideString(format("clip: ", CE_INT(g_pLocalPlayer->weapon(), netvar.m_iClip1)));
        if (local->m_iClassID() == CL_CLASS(CTFMinigun))
            AddSideString(format("Weapon state: ", CE_INT(local, netvar.iWeaponState)));
        AddSideString(format("ItemDefinitionIndex: ", CE_INT(local, netvar.iItemDefinitionIndex)));
        AddSideString(format("Maxspeed: ", CE_FLOAT(LOCAL_E, netvar.m_flMaxspeed)));
        /*AddSideString(colors::white, "Weapon: %s [%i]",
        RAW_ENT(g_pLocalPlayer->weapon())->GetClientClass()->GetName(),
        g_pLocalPlayer->weapon()->m_iClassID());
        //AddSideString(colors::white, "flNextPrimaryAttack: %f",
        CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flNextPrimaryAttack));
        //AddSideString(colors::white, "nTickBase: %f",
        (float)(CE_INT(g_pLocalPlayer->entity, netvar.nTickBase)) *
        gvars->interval_per_tick); AddSideString(colors::white, "CanShoot: %i",
        CanShoot());
        //AddSideString(colors::white, "Damage: %f",
        CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargedDamage)); if (TF2)
        AddSideString(colors::white, "DefIndex: %i",
        CE_INT(g_pLocalPlayer->weapon(), netvar.iItemDefinitionIndex));
        //AddSideString(colors::white, "GlobalVars: 0x%08x", gvars);
        //AddSideString(colors::white, "realtime: %f", gvars->realtime);
        //AddSideString(colors::white, "interval_per_tick: %f",
        gvars->interval_per_tick);
        //if (TF2) AddSideString(colors::white, "ambassador_can_headshot: %i",
        (gvars->curtime - CE_FLOAT(g_pLocalPlayer->weapon(),
        netvar.flLastFireTime)) > 0.95); AddSideString(colors::white,
        "WeaponMode: %i", GetWeaponMode(g_pLocalPlayer->entity));
        AddSideString(colors::white, "ToGround: %f",
        DistanceToGround(g_pLocalPlayer->v_Origin));
        AddSideString(colors::white, "ServerTime: %f",
        CE_FLOAT(g_pLocalPlayer->entity, netvar.nTickBase) *
        g_GlobalVars->interval_per_tick); AddSideString(colors::white, "CurTime:
        %f", g_GlobalVars->curtime); AddSideString(colors::white, "FrameCount:
        %i", g_GlobalVars->framecount); float speed, gravity;
        GetProjectileData(g_pLocalPlayer->weapon(), speed, gravity);
        AddSideString(colors::white, "ALT: %i",
        g_pLocalPlayer->bAttackLastTick); AddSideString(colors::white, "Speed:
        %f", speed); AddSideString(colors::white, "Gravity: %f", gravity);
        AddSideString(colors::white, "CIAC: %i", *(bool*)(RAW_ENT(LOCAL_W) +
        2380)); if (TF2) AddSideString(colors::white, "Melee: %i",
        vfunc<bool(*)(IClientEntity*)>(RAW_ENT(LOCAL_W), 1860 / 4,
        0)(RAW_ENT(LOCAL_W))); if (TF2) AddSideString(colors::white, "Bucket:
        %.2f", *(float*)((uintptr_t)RAW_ENT(LOCAL_W) + 2612u));
        //if (TF2C) AddSideString(colors::white, "Seed: %i",
        *(int*)(sharedobj::client->lmap->l_addr + 0x00D53F68ul));
        //AddSideString(colors::white, "IsZoomed: %i", g_pLocalPlayer->bZoomed);
        //AddSideString(colors::white, "CanHeadshot: %i", CanHeadshot());
        //AddSideString(colors::white, "IsThirdPerson: %i",
        iinput->CAM_IsThirdPerson());
        //if (TF2C) AddSideString(colors::white, "Crits: %i", s_bCrits);
        //if (TF2C) AddSideString(colors::white, "CritMult: %i",
        RemapValClampedNC( CE_INT(LOCAL_E, netvar.iCritMult), 0, 255, 1.0, 6 ));
        for (int i = 0; i <= HIGHEST_ENTITY; ++i) {
            CachedEntity* e = ENTITY(i);
            if (CE_GOOD(e)) {
                if (e->m_Type() == EntityType::ENTITY_PROJECTILE) {
                    //logging::Info("Entity %i [%s]: V %.2f (X: %.2f, Y: %.2f,
        Z: %.2f) ACC %.2f (X: %.2f, Y: %.2f, Z: %.2f)", i,
        RAW_ENT(e)->GetClientClass()->GetName(), e->m_vecVelocity.Length(),
        e->m_vecVelocity.x, e->m_vecVelocity.y, e->m_vecVelocity.z,
        e->m_vecAcceleration.Length(), e->m_vecAcceleration.x,
        e->m_vecAcceleration.y, e->m_vecAcceleration.z);
                    AddSideString(colors::white, "Entity %i [%s]: V %.2f (X:
        %.2f, Y: %.2f, Z: %.2f) ACC %.2f (X: %.2f, Y: %.2f, Z: %.2f)", i,
        RAW_ENT(e)->GetClientClass()->GetName(), e->m_vecVelocity.Length(),
        e->m_vecVelocity.x, e->m_vecVelocity.y, e->m_vecVelocity.z,
        e->m_vecAcceleration.Length(), e->m_vecAcceleration.x,
        e->m_vecAcceleration.y, e->m_vecAcceleration.z);
                }
            }
        }//AddSideString(draw::white, draw::black, "???: %f",
        NET_FLOAT(g_pLocalPlayer->entity, netvar.test));
        //AddSideString(draw::white, draw::black, "VecPunchAngle: %f %f %f",
        pa.x, pa.y, pa.z);
        //draw::DrawString(10, y, draw::white, draw::black, false,
        "VecPunchAngleVel: %f %f %f", pav.x, pav.y, pav.z);
        //y += 14;
        //AddCenterString(fonts::font_handle,
        input->GetAnalogValue(AnalogCode_t::MOUSE_X),
        input->GetAnalogValue(AnalogCode_t::MOUSE_Y), draw::white,
        L"S\u0FD5");*/
    }
}

#endif

void generate_schema()
{
    std::ifstream in("tf/scripts/items/items_game.txt");
    std::string outS((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::ofstream out(paths::getDataPath("/items_game.txt"));
    std::regex a("\"equip_regions?\".*?\".*?\"");
    std::regex b("\"equip_regions?\"\\s*?\\n\\s*?\\{[\\s\\S\\n]*?\\}");
    outS = std::regex_replace(outS, a, "");
    out << std::regex_replace(outS, b, "");
    out.close();
    logging::Info("Generating complete!");
}
static CatCommand generateschema("schema_generate", "Generate custom schema", generate_schema);

bool InitSchema(const char *fileName, const char *pathID, CUtlVector<CUtlString> *pVecErrors /* = NULL */)
{
    static auto GetItemSchema = reinterpret_cast<void *(*) (void)>(gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC ? 8B 1D ? ? ? ? 85 DB 89 D8"));

    static auto BInitTextBuffer = reinterpret_cast<bool (*)(void *, CUtlBuffer &, CUtlVector<CUtlString> *)>(gSignatures.GetClientSignature("55 89 E5 57 56 53 8D 9D ? ? ? ? 81 EC ? ? ? ? 8B 7D ? 89 1C 24 "));
    void *schema                = (void *) ((uintptr_t) GetItemSchema() + 0x4);

    // Read the raw data
    CUtlBuffer bufRawData;
    bool bReadFileOK = g_IFileSystem->ReadFile(fileName, pathID, bufRawData);
    // Log something here if bReadFileOK is false

    if (!bReadFileOK)
    {
        logging::Info(("Failed reading item schema from " + std::string(fileName)).c_str());
        return false;
    }

    else
        logging::Info(("Read item schema from " + std::string(fileName)).c_str());

    // Wrap it with a text buffer reader
    CUtlBuffer bufText(bufRawData.Base(), bufRawData.TellPut(), CUtlBuffer::READ_ONLY | CUtlBuffer::TEXT_BUFFER);

    // Use the standard init path
    return BInitTextBuffer(schema, bufText, pVecErrors);
}

void Schema_Reload()
{
    logging::Info("Loading item schema...");
    bool ret = InitSchema(paths::getDataPath("/items_game.txt").c_str(), nullptr, nullptr);
    logging::Info("Loading %s", ret ? "Successful" : "Unsuccessful");
}

CatCommand schema("schema", "Load custom schema", Schema_Reload);

CatCommand update_gui_color("gui_color_update", "Update the GUI Color",
                            []()
                            {
                                hack::command_stack().push("cat set zk.style.tab-button.color.selected.background 446498ff;cat set zk.style.tab-button.color.separator 446498ff;cat set zk.style.tab-button.color.hover.underline 446498ff;cat set zk.style.tab-button.color.selected.underline 446498ff;cat set zk.style.tooltip.border 446498ff;cat set zk.style.box.color.border 446498ff;cat set zk.style.color-preview.color.border 446498ff;cat set zk.style.modal-container.color.border 446498ff;cat set zk.style.tab-selection.color.border 446498ff;cat set zk.style.table.color.border 446498ff;cat set zk.style.checkbox.color.border 446498ff;cat set zk.style.checkbox.color.checked 446498ff;cat set zk.style.checkbox.color.hover 00a098ff;cat set zk.style.input.color.border 446498ff;cat set zk.style.input.key.color.border 446498ff;cat set zk.style.input.select.border 446498ff;cat set zk.style.input.slider.color.handle_border 446498ff;cat set zk.style.input.slider.color.bar 446498ff;cat set zk.style.input.text.color.border.active 42BC99ff");
                                hack::command_stack().push("cat set zk.style.input.text.color.border.inactive 446498ff;cat set zk.style.tree-list-entry.color.lines 42BC99ff;cat set zk.style.task.color.background.hover 446498ff;cat set zk.style.task.color.border 446498ff;cat set zk.style.taskbar.color.border 446498ff;cat set zk.style.window.color.border 446498ff;cat set zk.style.window-close-button.color.border 446498ff;cat set zk.style.window-header.color.background.active 446498ff;cat set zk.style.window-header.color.border.inactive 446498ff;cat set zk.style.window-header.color.border.active 446498ff");
                            });

CatCommand name("name_set", "Immediate name change",
                [](const CCommand &args)
                {
                    if (args.ArgC() < 2)
                    {
                        logging::Info("Set a name, silly");
                        return;
                    }
                    if (g_Settings.bInvalid)
                    {
                        logging::Info("Only works ingame!");
                        return;
                    }
                    std::string new_name(args.ArgS());
                    ReplaceSpecials(new_name);
                    NET_SetConVar setname("name", new_name.c_str());
                    INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
                    if (ch)
                    {
                        setname.SetNetChannel(ch);
                        setname.SetReliable(false);
                        ch->SendNetMsg(setname, false);
                    }
                });
CatCommand set_value("set", "Set value",
                     [](const CCommand &args)
                     {
                         if (args.ArgC() < 2)
                             return;
                         ConVar *var = g_ICvar->FindVar(args.Arg(1));
                         if (!var)
                             return;
                         std::string value(args.Arg(2));
                         ReplaceSpecials(value);
                         var->m_fMaxVal = 999999999.9f;
                         var->m_fMinVal = -999999999.9f;
                         var->SetValue(value.c_str());
                         logging::Info("Set '%s' to '%s'", args.Arg(1), value.c_str());
                     });
CatCommand get_value("get", "Set value",
                     [](const CCommand &args)
                     {
                         if (args.ArgC() < 2)
                             return;
                         ConVar *var = g_ICvar->FindVar(args.Arg(1));
                         if (!var)
                             return;
                         logging::Info("'%s': '%s'", args.Arg(1), var->GetString());
                     });
CatCommand say_lines("say_lines", "Say with newlines (\\n)",
                     [](const CCommand &args)
                     {
                         std::string message(args.ArgS());
                         ReplaceSpecials(message);
                         std::string cmd = format("say ", message);
                         g_IEngine->ServerCmd(cmd.c_str());
                     });
CatCommand disconnect("disconnect", "Disconnect with custom reason",
                      [](const CCommand &args)
                      {
                          INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
                          if (!ch)
                              return;
                          std::string string = args.ArgS();
                          ReplaceSpecials(string);
                          ch->Shutdown(string.c_str());
                      });

CatCommand disconnect_vac("disconnect_vac", "Disconnect (fake VAC)",
                          []()
                          {
                              INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
                              if (!ch)
                                  return;
                              ch->Shutdown("VAC banned from secure server\n");
                          });

// Netvars stuff
void DumpRecvTable(CachedEntity *ent, RecvTable *table, int depth, const char *ft, unsigned acc_offset)
{
    bool forcetable = ft && strlen(ft);
    if (!forcetable || !strcmp(ft, table->GetName()))
        logging::Info("==== TABLE: %s", table->GetName());
    for (int i = 0; i < table->GetNumProps(); ++i)
    {
        RecvProp *prop = table->GetProp(i);
        if (!prop)
            continue;
        if (prop->GetDataTable())
        {
            DumpRecvTable(ent, prop->GetDataTable(), depth + 1, ft, acc_offset + prop->GetOffset());
        }
        if (forcetable && strcmp(ft, table->GetName()))
            continue;
        switch (prop->GetType())
        {
        case SendPropType::DPT_Float:
            logging::Info("TABLE %s IN DEPTH %d: %s [0x%04x] = %f", table ? table->GetName() : "none", depth, prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset()));
            break;
        case SendPropType::DPT_Int:
            logging::Info("TABLE %s IN DEPTH %d: %s [0x%04x] = %i | %u | %hd | %hu", table ? table->GetName() : "none", depth, prop->GetName(), prop->GetOffset(), CE_INT(ent, acc_offset + prop->GetOffset()), CE_VAR(ent, acc_offset + prop->GetOffset(), unsigned int), CE_VAR(ent, acc_offset + prop->GetOffset(), short), CE_VAR(ent, acc_offset + prop->GetOffset(), unsigned short));
            break;
        case SendPropType::DPT_String:
            logging::Info("TABLE %s IN DEPTH %d: %s [0x%04x] = 'not yet supported'", table ? table->GetName() : "none", depth, prop->GetName(), prop->GetOffset());
            break;
        case SendPropType::DPT_Vector:
            logging::Info("TABLE %s IN DEPTH %d: %s [0x%04x] = (%f, %f, %f)", table ? table->GetName() : "none", depth, prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset()), CE_FLOAT(ent, acc_offset + prop->GetOffset() + 4), CE_FLOAT(ent, acc_offset + prop->GetOffset() + 8));
            break;
        case SendPropType::DPT_VectorXY:
            logging::Info("TABLE %s IN DEPTH %d: %s [0x%04x] = (%f, %f)", table ? table->GetName() : "none", depth, prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset()), CE_FLOAT(ent, acc_offset + prop->GetOffset() + 4));
            break;
            // There are some missing things here
        default:
            break;
        }
    }
    if (!ft || !strcmp(ft, table->GetName()))
        logging::Info("==== END OF TABLE: %s IN DEAPTH %d", table->GetName(), depth);
}

// CatCommand to dumb netvar info
static CatCommand dump_vars("debug_dump_netvars", "Dump netvars of entity",
                            [](const CCommand &args)
                            {
                                if (args.ArgC() < 2)
                                    return;
                                if (!atoi(args[1]))
                                    return;
                                int idx           = atoi(args[1]);
                                CachedEntity *ent = ENTITY(idx);
                                if (CE_BAD(ent))
                                    return;
                                ClientClass *clz = RAW_ENT(ent)->GetClientClass();
                                logging::Info("Entity %i: %s", ent->m_IDX, clz->GetName());
                                const char *ft = (args.ArgC() > 1 ? args[2] : 0);
                                DumpRecvTable(ent, clz->m_pRecvTable, 0, ft, 0);
                            });
static CatCommand dump_vars_by_name("debug_dump_netvars_name", "Dump netvars of entity with target name",
                                    [](const CCommand &args)
                                    {
                                        if (args.ArgC() < 2)
                                            return;
                                        std::string name(args.Arg(1));
                                        for (auto const &ent : entity_cache::valid_ents)
                                        {

                                            ClientClass *clz = RAW_ENT(ent)->GetClientClass();
                                            if (!clz)
                                                continue;
                                            std::string clazz_name(clz->GetName());
                                            if (clazz_name.find(name) == clazz_name.npos)
                                                continue;
                                            logging::Info("Entity %i: %s", ent->m_IDX, clz->GetName());
                                            const char *ft = (args.ArgC() > 1 ? args[2] : 0);
                                            DumpRecvTable(ent, clz->m_pRecvTable, 0, ft, 0);
                                        }
                                    });

static CatCommand debug_print_weaponid("debug_weaponid", "Print the weapon IDs of all currently equiped weapons",
                                       [](const CCommand &)
                                       {
                                           // Invalid player
                                           if (CE_BAD(LOCAL_E))
                                               return;
                                           int *hWeapons = &CE_INT(LOCAL_E, netvar.hMyWeapons);
                                           // Go through the handle array and search for the item
                                           for (int i = 0; hWeapons[i]; ++i)
                                           {
                                               if (IDX_BAD(HandleToIDX(hWeapons[i])))
                                                   continue;
                                               // Get the weapon
                                               CachedEntity *weapon = ENTITY(HandleToIDX(hWeapons[i]));
                                               // Print weaponid
                                               logging::Info("weapon %i: %i", i, re::C_TFWeaponBase::GetWeaponID(RAW_ENT(weapon)));
                                           }
                                       });

#if ENABLE_VISUALS && !ENFORCE_STREAM_SAFETY
// This makes us able to see enemy class and status in scoreboard and player panel
/*
static std::unique_ptr<BytePatch> patch_playerpanel;
static std::unique_ptr<BytePatch> patch_scoreboard1;
static std::unique_ptr<BytePatch> patch_scoreboard2;
static std::unique_ptr<BytePatch> patch_scoreboard3;
*/

// Credits to UNKN0WN
namespace ScoreboardColoring
{
static std::unique_ptr<BytePatch> patch_scoreboardcolor1;
static std::unique_ptr<BytePatch> patch_scoreboardcolor2;
uintptr_t addr1 = 0;
uintptr_t addr2 = 0;

// Colors taken from client.so
Color &GetPlayerColor(int idx, int team, bool dead = false)
{
    static Color returnColor(0, 0, 0, 0);

    int col_red[] = { scc ? int(colors::red.r * 255) : 255, scc ? int(colors::red.g * 255) : 64, scc ? int(colors::red.b * 255) : 64 };
    int col_blu[] = { scc ? int(colors::blu.r * 255) : 153, scc ? int(colors::blu.g * 255) : 204, scc ? int(colors::blu.b * 255) : 255 };

    switch (team)
    {
    case TEAM_RED:
        returnColor.SetColor(col_red[0], col_red[1], col_red[2], 255);
        break;
    case TEAM_BLU:
        returnColor.SetColor(col_blu[0], col_blu[1], col_blu[2], 255);
        break;
    default:
        returnColor.SetColor(245, 229, 196, 255);
    }

    player_info_s pinfo;
    if (GetPlayerInfo(idx, &pinfo))
    {
        rgba_t cust = playerlist::Color(pinfo.friendsID);
        if (cust != colors::empty)
            returnColor.SetColor(cust.r * 255, cust.g * 255, cust.b * 255, 255);
    }

    if (dead)
        for (int i = 0; i < 3; ++i)
            returnColor[i] /= 1.5f;

    return returnColor;
}

// This gets playerIndex with assembly magic, then sets a Color for the scoreboard entries
Color &GetTeamColor(void *, int team)
{
    int playerIndex;
    __asm__("mov %%esi, %0" : "=r"(playerIndex));
    return GetPlayerColor(playerIndex, team);
}

// This is some assembly magic in order to get playerIndex and team from already existing variables and then set scoreboard entries
Color &GetDeadPlayerColor()
{
    int playerIndex;
    int team;
    __asm__("mov %%esi, %0" : "=r"(playerIndex));
    team = g_pPlayerResource->GetTeam(playerIndex);
    return GetPlayerColor(playerIndex, team, true);
}

static InitRoutine init(
    []()
    {
        // 012BA7E4
        addr1 = gSignatures.GetClientSignature("89 04 24 FF 92 ? ? ? ? 8B 00") + 3;
        // 012BA105
        addr2 = gSignatures.GetClientSignature("75 1B 83 FB 02") + 2;
        if (addr1 == 3 || addr2 == 2)
            return;
        logging::Info("Patching scoreboard colors");

        // Used to Detour, we need to detour at two parts in order to do this properly
        auto relAddr1 = ((uintptr_t) GetTeamColor - (uintptr_t) addr1) - 5;
        auto relAddr2 = ((uintptr_t) GetDeadPlayerColor - (uintptr_t) addr2) - 5;

        // Construct BytePatch1
        std::vector<unsigned char> patch1 = { 0xE8 };
        for (size_t i = 0; i < sizeof(uintptr_t); ++i)
            patch1.push_back(((unsigned char *) &relAddr1)[i]);
        for (int i = patch1.size(); i < 6; ++i)
            patch1.push_back(0x90);

        // Construct BytePatch2
        std::vector<unsigned char> patch2 = { 0xE8 };
        for (size_t i = 0; i < sizeof(uintptr_t); ++i)
            patch2.push_back(((unsigned char *) &relAddr2)[i]);
        patch2.push_back(0x8B);
        patch2.push_back(0x00);
        for (int i = patch2.size(); i < 27; ++i)
            patch2.push_back(0x90);

        patch_scoreboardcolor1 = std::make_unique<BytePatch>(addr1, patch1);
        patch_scoreboardcolor2 = std::make_unique<BytePatch>(addr2, patch2);

        // Patch!
        patch_scoreboardcolor1->Patch();
        patch_scoreboardcolor2->Patch();

        EC::Register(
            EC::LevelInit,
            []()
            {
                // Remove truce status
                setTruce(false);
            },
            "truce_reset");
    });
} // namespace ScoreboardColoring

typedef void (*UpdateLocalPlayerVisionFlags_t)();
UpdateLocalPlayerVisionFlags_t UpdateLocalPlayerVisionFlags_fn;

int *g_nLocalPlayerVisionFlags;
int *g_nLocalPlayerVisionFlagsWeaponsCheck;
// If you wish then change this to some other flag you want to apply/remove
constexpr int PYROVISION = 1;

static settings::Int force_pyrovision("visual.force-pyrovision", "0");

void UpdateLocalPlayerVisionFlags()
{
    UpdateLocalPlayerVisionFlags_fn();
    if (!force_pyrovision)
        return;
    if (*force_pyrovision == 2)
    {
        *g_nLocalPlayerVisionFlags &= ~PYROVISION;
        *g_nLocalPlayerVisionFlagsWeaponsCheck &= ~PYROVISION;
    }
    else
    {
        *g_nLocalPlayerVisionFlags |= PYROVISION;
        *g_nLocalPlayerVisionFlagsWeaponsCheck |= PYROVISION;
    }
}
#define access_ptr(p, i) ((unsigned char *) &p)[i]

static InitRoutine init_pyrovision(
    []()
    {
        uintptr_t addr                        = gSignatures.GetClientSignature("8B 35 ? ? ? ? 75 27");
        g_nLocalPlayerVisionFlags             = *reinterpret_cast<int **>(addr + 2);
        g_nLocalPlayerVisionFlagsWeaponsCheck = *reinterpret_cast<int **>(addr + 8 + 2);
        addr                                  = gSignatures.GetClientSignature("C7 04 24 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8");
        if (!addr)
            return;
        addr += 17;
        UpdateLocalPlayerVisionFlags_fn = UpdateLocalPlayerVisionFlags_t(e8call_direct(addr));

        auto relAddr = ((uintptr_t) UpdateLocalPlayerVisionFlags - (uintptr_t) addr) - 5;

        static BytePatch patch{ addr, { 0xE8, access_ptr(relAddr, 0), access_ptr(relAddr, 1), access_ptr(relAddr, 2), access_ptr(relAddr, 3) } };
        patch.Patch();
        EC::Register(
            EC::Shutdown, []() { patch.Shutdown(); }, "shutdown_pyrovis");
#if !ENFORCE_STREAM_SAFETY
        EC::Register(
            EC::Paint,
            []()
            {
                if (CE_GOOD(LOCAL_E))
                {
                    if (HasCondition<TFCond_HalloweenKartNoTurn>(LOCAL_E))
                        RemoveCondition<TFCond_HalloweenKartNoTurn>(LOCAL_E);
                }
            },
            "remove_cart_cond");
        static BytePatch cart_patch1(gSignatures.GetClientSignature, "0F 84 ? ? ? ? F3 0F 10 A2", 0x0, { 0x90, 0xE9 });
        static BytePatch cart_patch2(gSignatures.GetClientSignature, "0F 85 ? ? ? ? 89 F8 84 C0 75 72", 0x0, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });
        if (unlimit_bumpercart_movement)
        {
            cart_patch1.Patch();
            cart_patch2.Patch();
        }
        unlimit_bumpercart_movement.installChangeCallback(
            [](settings::VariableBase<bool> &, bool after)
            {
                if (after)
                {
                    cart_patch1.Patch();
                    cart_patch2.Patch();
                }
                else
                {
                    cart_patch1.Shutdown();
                    cart_patch2.Shutdown();
                }
            });
        EC::Register(
            EC::Shutdown,
            []()
            {
                cart_patch1.Shutdown();
                cart_patch2.Shutdown();
            },
            "cartpatch_shutdown");
        ping_reducer.installChangeCallback(
            [](settings::VariableBase<bool> &, bool)
            {
                static ConVar *cmdrate = g_ICvar->FindVar("cl_cmdrate");
                if (cmdrate == nullptr)
                {
                    cmdrate = g_ICvar->FindVar("cl_cmdrate");
                    return;
                }
            });
#endif
    });
#endif

static CatCommand print_eye_diff("debug_print_eye_diff", "debug", []() { logging::Info("%f", g_pLocalPlayer->v_Eye.z - LOCAL_E->m_vecOrigin().z); });
void Shutdown()
{
#if ENABLE_VISUALS && !ENFORCE_STREAM_SAFETY
    // unpatching local player
    render_zoomed = false;
    /*
    patch_playerpanel->Shutdown();
    patch_scoreboard1->Shutdown();
    patch_scoreboard2->Shutdown();
    patch_scoreboard3->Shutdown();
    */
    if (ScoreboardColoring::addr1 == 3 || ScoreboardColoring::addr2 == 2)
        return;

    ScoreboardColoring::patch_scoreboardcolor1->Shutdown();
    ScoreboardColoring::patch_scoreboardcolor2->Shutdown();
#endif
}

static ProxyFnHook cyoa_anim_hook{};

void cyoaview_nethook(const CRecvProxyData *data, void *pPlayer, void *out)
{
    int value      = data->m_Value.m_Int;
    int *value_out = (int *) out;
    if (!fix_cyoaanim)
    {
        *value_out = value;
        return;
    }
    // Mark as not doing cyoa thing
    if (CE_BAD(LOCAL_E) || pPlayer != RAW_ENT(LOCAL_E))
        *value_out = false;
}

// This function does two things.
// 1. It patches the check that's supposed to update the "isWaitCommandEnabled" on the cvar buffer to always skip
// 2. It sets the wait command to enabled
// The Former is the main logic, the latter is so you don't accidentally perma disable it either
inline void force_wait_func(bool after)
{
    static auto enable_wait_signature = gSignatures.GetEngineSignature("74 ? A2 ? ? ? ? C7 44 24 ? 01 00 00 00");
    // Jump if not overflow, aka always jump in this case
    static BytePatch patch_wait(enable_wait_signature, { 0x71 });
    if (after)
    {
        // Enable the wait command
        int **enable_wait = (int **) (enable_wait_signature + 3);

        BytePatch::mprotectAddr((uintptr_t) enable_wait, 4, PROT_READ | PROT_WRITE | PROT_EXEC);
        BytePatch::mprotectAddr((uintptr_t) *enable_wait, 4, PROT_READ | PROT_WRITE | PROT_EXEC);

        **enable_wait = true;
        BytePatch::mprotectAddr((uintptr_t) enable_wait, 4, PROT_EXEC);

        patch_wait.Patch();
    }
    else
        patch_wait.Shutdown();
}

void callback_force_wait(settings::VariableBase<bool> &, bool after)
{
    force_wait_func(after);
}

static InitRoutine init(
    []()
    {
        HookNetvar({ "DT_TFPlayer", "m_bViewingCYOAPDA" }, cyoa_anim_hook, cyoaview_nethook);

        force_wait.installChangeCallback(callback_force_wait);
        force_wait_func(true);

        teammatesPushaway = g_ICvar->FindVar("tf_avoidteammates_pushaway");
        EC::Register(EC::Shutdown, Shutdown, "draw_local_player", EC::average);
        EC::Register(EC::CreateMove, CreateMove, "cm_misc_hacks", EC::average);
        EC::Register(EC::CreateMoveWarp, CreateMove, "cmw_misc_hacks", EC::average);
#if ENABLE_VISUALS
        EC::Register(EC::Draw, Draw, "draw_misc_hacks", EC::average);
#if !ENFORCE_STREAM_SAFETY
        if (render_zoomed)
            tryPatchLocalPlayerShouldDraw(true);
        /*render_zoomed.installChangeCallback([](settings::VariableBase<bool> &, bool after) { tryPatchLocalPlayerShouldDraw(after); });
        patch_playerpanel     = std::make_unique<BytePatch>(gSignatures.GetClientSignature, "0F 94 45 ? 85 C0 0F 8E", 0x0, std::vector<unsigned char>{ 0xC6, 0x45, 0xDF, 0x01 });
        uintptr_t addr_scrbrd = gSignatures.GetClientSignature("8B 10 89 74 24 04 89 04 24 FF 92 ? ? ? ? 83 F8 02 75 09");

        // Address to the function we need to jump to
        uintptr_t target_addr = e8call_direct(gSignatures.GetClientSignature("E8 ? ? ? ? 83 FE 2D"));
        uintptr_t rel_addr    = ((uintptr_t) target_addr - ((uintptr_t) addr_scrbrd + 2)) - 5;

        patch_scoreboard1 = std::make_unique<BytePatch>(addr_scrbrd, std::vector<unsigned char>{ 0xEB, 0x31, 0xE8, foffset(rel_addr, 0), foffset(rel_addr, 1), foffset(rel_addr, 2), foffset(rel_addr, 3), 0xE9, 0xC9, 0x06, 0x00, 0x00 });
        patch_scoreboard2 = std::make_unique<BytePatch>(addr_scrbrd + 0xA0, std::vector<unsigned char>{ 0xE9, 0x5D, 0xFF, 0xFF, 0xFF });
        patch_scoreboard3 = std::make_unique<BytePatch>(addr_scrbrd + 0x84A, std::vector<unsigned char>{ 0x8f, 0xFE });
        patch_playerpanel->Patch();
        patch_scoreboard1->Patch();
        patch_scoreboard2->Patch();
        patch_scoreboard3->Patch();
        */
        static BytePatch stealth_kill{ gSignatures.GetClientSignature, "84 C0 75 28 A1", 2, { 0x90, 0x90 } }; // stealth kill patch
        stealth_kill.Patch();
        static BytePatch cyoa_patch{ gSignatures.GetClientSignature, "75 ? 80 BB ? ? ? ? 00 74 ? A1 ? ? ? ? 8B 10 C7 44 24", 0, { 0xEB } };
        cyoa_patch.Patch();
        EC::Register(
            EC::Shutdown,
            []()
            {
                stealth_kill.Shutdown();
                cyoa_patch.Shutdown();
                tryPatchLocalPlayerShouldDraw(false);
                force_wait_func(false);
            },
            "shutdown_stealthkill");
        dont_hide_stealth_kills.installChangeCallback(
            [](settings::VariableBase<bool> &, bool after)
            {
                if (after)
                    stealth_kill.Patch();
                else
                    stealth_kill.Shutdown();
            });
#endif
#endif
    });
} // namespace hacks::shared::misc

/*void DumpRecvTable(CachedEntity* ent, RecvTable* table, int depth, const char*
ft, unsigned acc_offset) { bool forcetable = ft && strlen(ft); if (!forcetable
|| !strcmp(ft, table->GetName())) logging::Info("==== TABLE: %s",
table->GetName()); for (int i = 0; i < table->GetNumProps(); ++i) { RecvProp*
prop = table->GetProp(i); if (!prop) continue; if (prop->GetDataTable()) {
            DumpRecvTable(ent, prop->GetDataTable(), depth + 1, ft, acc_offset +
prop->GetOffset());
        }
        if (forcetable && strcmp(ft, table->GetName())) continue;
        switch (prop->GetType()) {
        case SendPropType::DPT_Float:
            logging::Info("%s [0x%04x] = %f", prop->GetName(),
prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset())); break; case
SendPropType::DPT_Int: logging::Info("%s [0x%04x] = %i | %u | %hd | %hu",
prop->GetName(), prop->GetOffset(), CE_INT(ent, acc_offset + prop->GetOffset()),
CE_VAR(ent, acc_offset +  prop->GetOffset(), unsigned int), CE_VAR(ent,
acc_offset + prop->GetOffset(), short), CE_VAR(ent, acc_offset +
prop->GetOffset(), unsigned short)); break; case SendPropType::DPT_String:
            logging::Info("%s [0x%04x] = %s", prop->GetName(),
prop->GetOffset(), CE_VAR(ent, prop->GetOffset(), char*)); break; case
SendPropType::DPT_Vector: logging::Info("%s [0x%04x] = (%f, %f, %f)",
prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset +
prop->GetOffset()), CE_FLOAT(ent, acc_offset + prop->GetOffset() + 4),
CE_FLOAT(ent, acc_offset + prop->GetOffset() + 8)); break; case
SendPropType::DPT_VectorXY: logging::Info("%s [0x%04x] = (%f, %f)",
prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset +
prop->GetOffset()), CE_FLOAT(ent,acc_offset +  prop->GetOffset() + 4)); break;
        }

    }
    if (!ft || !strcmp(ft, table->GetName()))
        logging::Info("==== END OF TABLE: %s", table->GetName());
}

void CC_DumpVars(const CCommand& args) {
    if (args.ArgC() < 1) return;
    if (!atoi(args[1])) return;
    int idx = atoi(args[1]);
    CachedEntity* ent = ENTITY(idx);
    if (CE_BAD(ent)) return;
    ClientClass* clz = RAW_ENT(ent)->GetClientClass();
    logging::Info("Entity %i: %s", ent->m_IDX, clz->GetName());
    const char* ft = (args.ArgC() > 1 ? args[2] : 0);
    DumpRecvTable(ent, clz->m_pRecvTable, 0, ft, 0);
}*/
