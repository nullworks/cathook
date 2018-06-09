/*
 * Misc.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: nullifiedcat
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <link.h>
#include <hacks/AntiAim.hpp>

#include "core/sharedobj.hpp"

#include "hack.hpp"
#include "common.hpp"

namespace hacks
{
namespace shared
{
namespace misc
{

static CatVar debug_info(CV_SWITCH, XORSTR("debug_info"), XORSTR("0"), XORSTR("Debug info"),
                         XORSTR("Shows some debug info in-game"));
static CatVar flashlight_spam(CV_SWITCH, XORSTR("flashlight"), XORSTR("0"), XORSTR("Flashlight spam"),
                              XORSTR("HL2DM flashlight spam"));
static CatVar
    auto_balance_spam(CV_SWITCH, XORSTR("request_balance_spam"), XORSTR("0"),
                      XORSTR("Inf Auto Balance Spam"),
                      XORSTR("Use to send a autobalance request to the server that "
                      "doesnt prevent you from using it again\nCredits to "
                      "Blackfire"));
static CatVar
    anti_afk(CV_SWITCH, XORSTR("anti_afk"), XORSTR("0"), XORSTR("Anti-AFK"),
             XORSTR("Sends random commands to prevent being kicked from server"));
static CatVar auto_strafe(CV_SWITCH, XORSTR("auto_strafe"), XORSTR("0"), XORSTR("Auto-Strafe"),
                          XORSTR("Automaticly airstrafes for you."));
static CatVar
    render_zoomed(CV_SWITCH, XORSTR("render_zoomed"), XORSTR("0"),
                  XORSTR("Render model when zoomed-in"),
                  XORSTR("Renders player model while being zoomed in as Sniper"));
static CatVar nopush_enabled(CV_SWITCH, XORSTR("nopush_enabled"), XORSTR("0"), XORSTR("No Push"),
                             XORSTR("Prevents other players from pushing you around."));

static CatVar no_homo(CV_SWITCH, XORSTR("no_homo"), XORSTR("1"), XORSTR("No Homo"), XORSTR("read if gay"));
// Taunting stuff
static CatVar tauntslide(CV_SWITCH, XORSTR("tauntslide"), XORSTR("0"), XORSTR("TF2C tauntslide"),
                         XORSTR("Allows moving and shooting while taunting"));
static CatVar tauntslide_tf2(CV_SWITCH, XORSTR("tauntslide_tf2"), XORSTR("0"), XORSTR("Tauntslide"),
                             XORSTR("Allows free movement while taunting with movable "
                             "taunts\nOnly works in tf2"));
static CatVar
    show_spectators(CV_SWITCH, XORSTR("show_spectators"), XORSTR("0"), XORSTR("Show spectators"),
                    XORSTR("Show who's spectating you\nonly works in valve servers"));
static CatVar god_mode(CV_SWITCH, XORSTR("godmode"), XORSTR("0"), XORSTR("no description"),
                       XORSTR("no description"));
void *C_TFPlayer__ShouldDraw_original = nullptr;

bool C_TFPlayer__ShouldDraw_hook(IClientEntity *thisptr)
{
    if (thisptr ==
            g_IEntityList->GetClientEntity(g_IEngine->GetLocalPlayer()) &&
        g_pLocalPlayer->bZoomed && thisptr)
    {
        // NET_INT(thisptr, netvar.iCond) &= ~(1 << TFCond_Zoomed);
        // bool result =
        // ((bool(*)(IClientEntity*))C_TFPlayer__ShouldDraw_original)(thisptr);
        // NET_INT(thisptr, netvar.iCond) |=  (1 << TFCond_Zoomed);
        return true;
    }
    else
    {
        return ((bool (*)(IClientEntity *)) C_TFPlayer__ShouldDraw_original)(
            thisptr);
    }
}

int last_number = 0;

float last_bucket = 0;

static CatCommand test_chat_print(
    XORSTR("debug_print_chat"), XORSTR("machine broke"), [](const CCommand &args) {
        CHudBaseChat *chat = (CHudBaseChat *) g_CHUD->FindElement(XORSTR("CHudChat"));
        if (chat)
        {
            std::unique_ptr<char> str(
                strfmt(XORSTR("\x07%06X[CAT]\x01 %s"), 0x4D7942, args.ArgS()));
            chat->Printf(str.get());
        }
        else
        {
            logging::Info(XORSTR("Chat is null!"));
        }
    });

// Use to send a autobalance request to the server that doesnt prevent you from
// using it again, Allowing infinite use of it.
void SendAutoBalanceRequest()
{ // Credits to blackfire
    if (!g_IEngine->IsInGame())
        return;
    KeyValues *kv = new KeyValues(XORSTR("AutoBalanceVolunteerReply"));
    kv->SetInt(XORSTR("response"), 1);
    g_IEngine->ServerCmdKeyValues(kv);
}
// Catcommand for above
CatCommand
    SendAutoBlRqCatCom(XORSTR("request_balance"), XORSTR("Request Infinite Auto-Balance"),
                       [](const CCommand &args) { SendAutoBalanceRequest(); });

void CreateMove()
{
#if not LAGBOT_MODE
    // Crithack
    static IClientEntity *localplayer, *weapon, *last_weapon = nullptr;
    static int tries, cmdn, md5seed, rseed, c, b;
    static crithack_saved_state state;
    static bool chc;
    static bool changed = false;

    if (g_pUserCmd->command_number)
        last_number = g_pUserCmd->command_number;

    static int last_checked_command_number    = 0;
    static IClientEntity *last_checked_weapon = nullptr;

    /*IF_GAME (IsTF2()) {
        if (crit_hack_next && CE_GOOD(LOCAL_E) && CE_GOOD(LOCAL_W) &&
       WeaponCanCrit() && RandomCrits()) {
            PROF_SECTION(CM_misc_crit_hack_prediction);
            weapon = RAW_ENT(LOCAL_W);
            // IsBaseCombatWeapon
            if (weapon &&
                vfunc<bool(*)(IClientEntity*)>(weapon, 1944 / 4, 0)(weapon)) {
                /*if (experimental_crit_hack.KeyDown()) {
                    if (!g_pUserCmd->command_number || critWarmup < 8) {
                        if (g_pUserCmd->buttons & IN_ATTACK) {
                            critWarmup++;
                        } else {
                            critWarmup = 0;
                        }
                        g_pUserCmd->buttons &= ~(IN_ATTACK);
                    }
                }*/ /*
                             if (g_pUserCmd->command_number &&
                 (last_checked_weapon !=
                 weapon || last_checked_command_number <
                 g_pUserCmd->command_number))
                 {
                                 tries = 0;
                                 cmdn = g_pUserCmd->command_number;
                                 chc = false;
                                 state.Save(weapon);
                                 while (!chc && tries < 4096) {
                                     md5seed = MD5_PseudoRandom(cmdn) &
                 0x7fffffff;
                                     rseed = md5seed;
                                     //float bucket =
                 *(float*)((uintptr_t)RAW_ENT(LOCAL_W)
                 + 2612u); *g_PredictionRandomSeed = md5seed; c = LOCAL_W->m_IDX
                 << 8;
                 b =
                 LOCAL_E->m_IDX; rseed = rseed ^ (b | c);
                                     *(float*)(weapon + 2872ul) = 0.0f;
                                     RandomSeed(rseed);
                                     chc =
                 vfunc<bool(*)(IClientEntity*)>(weapon, 1836
                 / 4,
                 0)(weapon); if (!chc) { tries++; cmdn++;
                                     }
                                 }
                                 last_checked_command_number = cmdn;
                                 last_checked_weapon = weapon;
                                 state.Load(weapon);
                                 last_bucket = state.bucket;
                                 if (chc) {
                                     found_crit_weapon = weapon;
                                     found_crit_number = cmdn;
                                 }
                             }
                             if (g_pUserCmd->buttons & (IN_ATTACK)) {
                                 if (found_crit_weapon == weapon &&
                 g_pUserCmd->command_number < found_crit_number) { if
                 (g_IInputSystem->IsButtonDown((ButtonCode_t)((int)experimental_crit_hack)))
                 { command_number_mod[g_pUserCmd->command_number] = cmdn;
                                     }
                                 }
                             }
                         }
                     }
                 }*/
    /*
    {
        PROF_SECTION(CM_misc_crit_hack_apply);
        if (!AllowAttacking()) g_pUserCmd->buttons &= ~IN_ATTACK;
    }*/
    // Spycrab stuff
    // TODO FIXME this should be moved out of here
    IF_GAME(IsTF2())
    {
        PROF_SECTION(CM_misc_hook_checks);
        static IClientEntity *localplayer = nullptr;
        localplayer =
            g_IEntityList->GetClientEntity(g_IEngine->GetLocalPlayer());
        if (render_zoomed && localplayer)
        {
            // Patchking local player
            void **vtable = *(void ***) (localplayer);
            if (vtable[offsets::ShouldDraw()] != C_TFPlayer__ShouldDraw_hook)
            {
                C_TFPlayer__ShouldDraw_original = vtable[offsets::ShouldDraw()];
                void *page = (void *) ((uintptr_t) vtable & ~0xFFF);
                mprotect(page, 0xFFF, PROT_READ | PROT_WRITE | PROT_EXEC);
                vtable[offsets::ShouldDraw()] =
                    (void *) C_TFPlayer__ShouldDraw_hook;
                mprotect(page, 0xFFF, PROT_READ | PROT_EXEC);
            }
        }
    }
    // AntiAfk That after a certian time without movement keys depressed, causes
    // random keys to be spammed for 1 second
    if (anti_afk)
    {

        // Time last idle
        static float afk_time_idle = 0;

        // If the timer exceeds 1 minute, jump and reset the timer
        if (g_GlobalVars->curtime - 60 > afk_time_idle)
        {

            // Send random commands
            g_pUserCmd->sidemove    = RandFloatRange(-450.0, 450.0);
            g_pUserCmd->forwardmove = RandFloatRange(-450.0, 450.0);
            g_pUserCmd->buttons     = rand();
            // Prevent attack command
            g_pUserCmd->buttons &= ~IN_ATTACK;

            // After 1 second we reset the idletime
            if (g_GlobalVars->curtime - 61 > afk_time_idle)
            {
                logging::Info(XORSTR("Finish anti-idle"));
                afk_time_idle = g_GlobalVars->curtime;
            }
        }
        else
        {
            // If the player uses a button, reset the timer
            if (g_pUserCmd->buttons & IN_FORWARD ||
                g_pUserCmd->buttons & IN_BACK ||
                g_pUserCmd->buttons & IN_MOVELEFT ||
                g_pUserCmd->buttons & IN_MOVERIGHT ||
                g_pUserCmd->buttons & IN_JUMP || !LOCAL_E->m_bAlivePlayer())
                afk_time_idle = g_GlobalVars->curtime;
        }
    }

    // Automaticly airstrafes in the air
    if (auto_strafe)
    {
        bool ground = CE_INT(g_pLocalPlayer->entity, netvar.iFlags) & (1 << 0);
        if (!ground)
        {
            if (g_pUserCmd->mousedx > 1 || g_pUserCmd->mousedx < -1)
            {
                g_pUserCmd->sidemove = g_pUserCmd->mousedx > 1 ? 450.f : -450.f;
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
            static bool flswitch = false;
            if (flswitch && !g_pUserCmd->impulse)
                g_pUserCmd->impulse = 100;
            flswitch                = !flswitch;
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
                    if (g_pUserCmd->buttons & IN_FORWARD)
                        forward += 450;
                    if (g_pUserCmd->buttons & IN_BACK)
                        forward -= 450;
                    if (g_pUserCmd->buttons & IN_MOVELEFT)
                        side -= 450;
                    if (g_pUserCmd->buttons & IN_MOVERIGHT)
                        side += 450;
                    g_pUserCmd->forwardmove = forward;
                    g_pUserCmd->sidemove    = side;

                    static QAngle camera_angle;
                    g_IEngine->GetViewAngles(camera_angle);

                    // Doesnt work with anti-aim as well as I hoped... I guess
                    // this is as far as I can go with such a simple tauntslide
                    if (!(hacks::shared::antiaim::enabled &&
                          hacks::shared::antiaim::yaw_mode &&
                          !(side || forward)))
                        g_pUserCmd->viewangles.y       = camera_angle[1];
                    g_pLocalPlayer->v_OrigViewangles.y = camera_angle[1];

                    // Use silent since we dont want to prevent the player from
                    // looking around
                    g_pLocalPlayer->bUseSilentAngles = true;
                }
            }
        }

        // Spams infinite autobalance spam function
        if (auto_balance_spam)
        {

            static float auto_balance_time = 0;
            if (g_GlobalVars->curtime - 0.15 > auto_balance_time)
            {

                SendAutoBalanceRequest();
                // Reset
                auto_balance_time = g_GlobalVars->curtime;
            }
        }

        // Simple No-Push through cvars
        static ConVar *pNoPush = g_ICvar->FindVar(XORSTR("tf_avoidteammates_pushaway"));
        if (nopush_enabled == pNoPush->GetBool())
            pNoPush->SetValue(!nopush_enabled);
    }
#endif
}

#if ENABLE_VISUALS

void DrawText()
{
    if (god_mode)
        for (int i = 0; i < 40000; i++)
        {
            g_ISurface->PlaySound(XORSTR("vo/demoman_cloakedspy03.mp3"));
            god_mode = 0;
        }
    if (!no_homo)
    {
        int width, height;
        g_IEngine->GetScreenSize(width, height);

        // Create steps from screen size
        int step = (height / 7);

        // Go through steps creating a rainbow screen
        for (int i = 1; i < 8; i++)
        {
            // Get Color and set opacity to %50
            colors::rgba_t gaybow = colors::FromHSL(
                fabs(sin((g_GlobalVars->curtime / 2.0f) + (i / 1.41241))) *
                    360.0f,
                0.85f, 0.9f);
            gaybow.a = .5;
            // Draw next step
            draw_api::draw_rect(0, step * (i - 1), width,
                                (step * i) - (step * (i - 1)), gaybow);
        }

        // int size_x;
        // FTGL_StringLength(string.data, fonts::font_main, &size_x);
        // FTGL_Draw(string.data, draw_point.x - size_x / 2,
        // draw_point.y,fonts::font_main, color);
    }
    if (show_spectators)
    {
        for (int i = 0; i < 32; i++)
        {
            // Assign the for loops tick number to an ent
            CachedEntity *ent = ENTITY(i);
            player_info_s info;
            if (!CE_BAD(ent) && ent != LOCAL_E &&
                ent->m_Type() == ENTITY_PLAYER &&
                (CE_INT(ent, netvar.hObserverTarget) & 0xFFF) ==
                    LOCAL_E->m_IDX &&
                CE_INT(ent, netvar.iObserverMode) >= 4 &&
                g_IEngine->GetPlayerInfo(i, &info))
            {
                auto observermode = XORSTR("N/A");
                switch (CE_INT(ent, netvar.iObserverMode))
                {
                case 4:
                    observermode = XORSTR("Firstperson");
                    break;
                case 5:
                    observermode = XORSTR("Thirdperson");
                    break;
                case 7:
                    observermode = XORSTR("Freecam");
                    break;
                }
                AddSideString(format(info.name, XORSTR(" "), observermode));
            }
        }
    }
    if (!debug_info)
        return;
    if (CE_GOOD(g_pLocalPlayer->weapon()))
    {
        AddSideString(format(XORSTR("Slot: "), re::C_BaseCombatWeapon::GetSlot(
                                           RAW_ENT(g_pLocalPlayer->weapon()))));
        AddSideString(
            format(XORSTR("Taunt Concept: "), CE_INT(LOCAL_E, netvar.m_iTauntConcept)));
        AddSideString(
            format(XORSTR("Taunt Index: "), CE_INT(LOCAL_E, netvar.m_iTauntIndex)));
        AddSideString(
            format(XORSTR("Sequence: "), CE_INT(LOCAL_E, netvar.m_nSequence)));
        AddSideString(format(XORSTR("Velocity: "), LOCAL_E->m_vecVelocity.x, ' ',
                             LOCAL_E->m_vecVelocity.y, ' ',
                             LOCAL_E->m_vecVelocity.z));
        AddSideString(format(XORSTR("Velocity3: "), LOCAL_E->m_vecVelocity.Length()));
        AddSideString(format(XORSTR("Velocity2: "), LOCAL_E->m_vecVelocity.Length2D()));
        AddSideString(XORSTR("NetVar Velocity"));
        Vector vel = CE_VECTOR(LOCAL_E, netvar.vVelocity);
        AddSideString(format(XORSTR("Velocity: "), vel.x, ' ', vel.y, ' ', vel.z));
        AddSideString(format(XORSTR("Velocity3: "), vel.Length()));
        AddSideString(format(XORSTR("Velocity2: "), vel.Length2D()));
        AddSideString(format(XORSTR("flSimTime: "),
                             LOCAL_E->var<float>(netvar.m_flSimulationTime)));
        if (g_pUserCmd)
            AddSideString(format(XORSTR("command_number: "), last_cmd_number));
        AddSideString(format(
            XORSTR("clip: "), CE_INT(g_pLocalPlayer->weapon(), netvar.m_iClip1)));
        /*AddSideString(colors::white, XORSTR("Weapon: %s [%i]"),
        RAW_ENT(g_pLocalPlayer->weapon())->GetClientClass()->GetName(),
        g_pLocalPlayer->weapon()->m_iClassID());
        //AddSideString(colors::white, XORSTR("flNextPrimaryAttack: %f"),
        CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flNextPrimaryAttack));
        //AddSideString(colors::white, XORSTR("nTickBase: %f"),
        (float)(CE_INT(g_pLocalPlayer->entity, netvar.nTickBase)) *
        gvars->interval_per_tick); AddSideString(colors::white, XORSTR("CanShoot: %i"),
        CanShoot());
        //AddSideString(colors::white, XORSTR("Damage: %f"),
        CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargedDamage)); if (TF2)
        AddSideString(colors::white, XORSTR("DefIndex: %i"),
        CE_INT(g_pLocalPlayer->weapon(), netvar.iItemDefinitionIndex));
        //AddSideString(colors::white, XORSTR("GlobalVars: 0x%08x"), gvars);
        //AddSideString(colors::white, XORSTR("realtime: %f"), gvars->realtime);
        //AddSideString(colors::white, XORSTR("interval_per_tick: %f"),
        gvars->interval_per_tick);
        //if (TF2) AddSideString(colors::white, XORSTR("ambassador_can_headshot: %i"),
        (gvars->curtime - CE_FLOAT(g_pLocalPlayer->weapon(),
        netvar.flLastFireTime)) > 0.95); AddSideString(colors::white,
        XORSTR("WeaponMode: %i"), GetWeaponMode(g_pLocalPlayer->entity));
        AddSideString(colors::white, XORSTR("ToGround: %f"),
        DistanceToGround(g_pLocalPlayer->v_Origin));
        AddSideString(colors::white, XORSTR("ServerTime: %f"),
        CE_FLOAT(g_pLocalPlayer->entity, netvar.nTickBase) *
        g_GlobalVars->interval_per_tick); AddSideString(colors::white, XORSTR("CurTime:
        %fXORSTR(", g_GlobalVars->curtime); AddSideString(colors::white, ")FrameCount:
        %iXORSTR(", g_GlobalVars->framecount); float speed, gravity;
        GetProjectileData(g_pLocalPlayer->weapon(), speed, gravity);
        AddSideString(colors::white, XORSTR("ALT: %i"),
        g_pLocalPlayer->bAttackLastTick); AddSideString(colors::white, XORSTR("Speed:
        %fXORSTR(", speed); AddSideString(colors::white, ")Gravity: %fXORSTR(", gravity);
        AddSideString(colors::white, XORSTR("CIAC: %i"), *(bool*)(RAW_ENT(LOCAL_W) +
        2380)); if (TF2) AddSideString(colors::white, XORSTR("Melee: %i"),
        vfunc<bool(*)(IClientEntity*)>(RAW_ENT(LOCAL_W), 1860 / 4,
        0)(RAW_ENT(LOCAL_W))); if (TF2) AddSideString(colors::white, XORSTR("Bucket:
        %.2fXORSTR(", *(float*)((uintptr_t)RAW_ENT(LOCAL_W) + 2612u));
        //if (TF2C) AddSideString(colors::white, XORSTR("Seed: %i"),
        *(int*)(sharedobj::client->lmap->l_addr + 0x00D53F68ul));
        //AddSideString(colors::white, XORSTR("IsZoomed: %i"), g_pLocalPlayer->bZoomed);
        //AddSideString(colors::white, XORSTR("CanHeadshot: %i"), CanHeadshot());
        //AddSideString(colors::white, XORSTR("IsThirdPerson: %i"),
        iinput->CAM_IsThirdPerson());
        //if (TF2C) AddSideString(colors::white, XORSTR("Crits: %i"), s_bCrits);
        //if (TF2C) AddSideString(colors::white, XORSTR("CritMult: %i"),
        RemapValClampedNC( CE_INT(LOCAL_E, netvar.iCritMult), 0, 255, 1.0, 6 ));
        for (int i = 0; i < HIGHEST_ENTITY; i++) {
            CachedEntity* e = ENTITY(i);
            if (CE_GOOD(e)) {
                if (e->m_Type() == EntityType::ENTITY_PROJECTILE) {
                    //logging::Info(XORSTR("Entity %i [%s]: V %.2f (X: %.2f, Y: %.2f,
        Z: %.2f) ACC %.2f (X: %.2f, Y: %.2f, Z: %.2f)XORSTR(", i,
        RAW_ENT(e)->GetClientClass()->GetName(), e->m_vecVelocity.Length(),
        e->m_vecVelocity.x, e->m_vecVelocity.y, e->m_vecVelocity.z,
        e->m_vecAcceleration.Length(), e->m_vecAcceleration.x,
        e->m_vecAcceleration.y, e->m_vecAcceleration.z);
                    AddSideString(colors::white, XORSTR("Entity %i [%s]: V %.2f (X:
        %.2f, Y: %.2f, Z: %.2f) ACC %.2f (X: %.2f, Y: %.2f, Z: %.2f)XORSTR(", i,
        RAW_ENT(e)->GetClientClass()->GetName(), e->m_vecVelocity.Length(),
        e->m_vecVelocity.x, e->m_vecVelocity.y, e->m_vecVelocity.z,
        e->m_vecAcceleration.Length(), e->m_vecAcceleration.x,
        e->m_vecAcceleration.y, e->m_vecAcceleration.z);
                }
            }
        }//AddSideString(draw::white, draw::black, XORSTR("???: %f"),
        NET_FLOAT(g_pLocalPlayer->entity, netvar.test));
        //AddSideString(draw::white, draw::black, XORSTR("VecPunchAngle: %f %f %f"),
        pa.x, pa.y, pa.z);
        //draw::DrawString(10, y, draw::white, draw::black, false,
        XORSTR("VecPunchAngleVel: %f %f %f"), pav.x, pav.y, pav.z);
        //y += 14;
        //AddCenterString(draw::font_handle,
        input->GetAnalogValue(AnalogCode_t::MOUSE_X),
        input->GetAnalogValue(AnalogCode_t::MOUSE_Y), draw::white,
        LXORSTR("S\u0FD5"));*/
    }
}

#endif

void Schema_Reload()
{
    logging::Info(XORSTR("Custom schema loading is not supported right now."));

    static uintptr_t InitSchema_s = gSignatures.GetClientSignature(
        XORSTR("55 89 E5 57 56 53 83 EC ? 8B 5D ? 8B 7D ? 8B 03 89 1C 24 FF 50 ? C7 "
        "04 24 ? ? ? ?"));
    typedef bool (*InitSchema_t)(void *, CUtlBuffer &, int);
    static InitSchema_t InitSchema   = (InitSchema_t) InitSchema_s;
    static uintptr_t GetItemSchema_s = gSignatures.GetClientSignature(
        XORSTR("55 89 E5 83 EC ? E8 ? ? ? ? C9 83 C0 ? C3 55 89 E5 8B 45 ?"));
    typedef void *(*GetItemSchema_t)(void);
    static GetItemSchema_t GetItemSchema = (GetItemSchema_t)
        GetItemSchema_s; //(*(uintptr_t*)GetItemSchema_s +GetItemSchema_s + 4);

    logging::Info(XORSTR("0x%08x 0x%08x"), InitSchema, GetItemSchema);
    void *itemschema = (void *) ((unsigned) GetItemSchema() + 4);
    void *data;
    char *path = strfmt(XORSTR("/opt/cathook/data/items_game.txt"));
    FILE *file = fopen(path, XORSTR("r"));
    delete[] path;
    fseek(file, 0L, SEEK_END);
    char buffer[5 * 1000 * 1000];
    size_t len = ftell(file);
    rewind(file);
    buffer[len + 1] = 0;
    fread(&buffer, sizeof(char), len, file);
    CUtlBuffer buf(&buffer, 5 * 1000 * 1000, 9);
    if (ferror(file) != 0)
    {
        logging::Info(XORSTR("Error loading file"));
        fclose(file);
        return;
    }
    fclose(file);
    logging::Info(XORSTR("0x%08x 0x%08x"), InitSchema, GetItemSchema);
    bool ret = InitSchema(GetItemSchema(), buf, 133769);
    logging::Info(XORSTR("Loading %s"), ret ? XORSTR("Successful") : XORSTR("Unsuccessful"));
}
CatCommand schema(XORSTR("schema"), XORSTR("Load custom schema"), Schema_Reload);

CatCommand name(XORSTR("name_set"), XORSTR("Immediate name change"), [](const CCommand &args) {
    if (args.ArgC() < 2)
    {
        logging::Info(XORSTR("Set a name, silly"));
        return;
    }
    if (g_Settings.bInvalid)
    {
        logging::Info(XORSTR("Only works ingame!"));
        return;
    }
    std::string new_name(args.ArgS());
    ReplaceString(new_name, XORSTR("\\n"), XORSTR("\n"));
    NET_SetConVar setname(XORSTR("name"), new_name.c_str());
    INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    if (ch)
    {
        setname.SetNetChannel(ch);
        setname.SetReliable(false);
        ch->SendNetMsg(setname, false);
    }
});
CatCommand set_value(XORSTR("set"), XORSTR("Set value"), [](const CCommand &args) {
    if (args.ArgC() < 2)
        return;
    ConVar *var = g_ICvar->FindVar(args.Arg(1));
    if (!var)
        return;
    std::string value(args.Arg(2));
    ReplaceString(value, XORSTR("\\n"), XORSTR("\n"));
    var->SetValue(value.c_str());
    logging::Info(XORSTR("Set '%s' to '%s'"), args.Arg(1), value.c_str());
});
CatCommand say_lines(XORSTR("say_lines"), XORSTR("Say with newlines (\\n)"),
                     [](const CCommand &args) {
                         std::string message(args.ArgS());
                         ReplaceString(message, XORSTR("\\n"), XORSTR("\n"));
                         std::string cmd = format(XORSTR("say "), message);
                         g_IEngine->ServerCmd(cmd.c_str());
                     });
CatCommand disconnect(XORSTR("disconnect"), XORSTR("Disconnect with custom reason"),
                      [](const CCommand &args) {
                          INetChannel *ch =
                              (INetChannel *) g_IEngine->GetNetChannelInfo();
                          if (!ch)
                              return;
                          ch->Shutdown(args.ArgS());
                      });

CatCommand disconnect_vac(XORSTR("disconnect_vac"), XORSTR("Disconnect (fake VAC)"), []() {
    INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    if (!ch)
        return;
    ch->Shutdown(XORSTR("VAC banned from secure server\n"));
});

// Netvars stuff
void DumpRecvTable(CachedEntity *ent, RecvTable *table, int depth,
                   const char *ft, unsigned acc_offset)
{
    bool forcetable = ft && strlen(ft);
    if (!forcetable || !strcmp(ft, table->GetName()))
        logging::Info(XORSTR("==== TABLE: %s"), table->GetName());
    for (int i = 0; i < table->GetNumProps(); i++)
    {
        RecvProp *prop = table->GetProp(i);
        if (!prop)
            continue;
        if (prop->GetDataTable())
        {
            DumpRecvTable(ent, prop->GetDataTable(), depth + 1, ft,
                          acc_offset + prop->GetOffset());
        }
        if (forcetable && strcmp(ft, table->GetName()))
            continue;
        switch (prop->GetType())
        {
        case SendPropType::DPT_Float:
            logging::Info(XORSTR("%s [0x%04x] = %f"), prop->GetName(),
                          prop->GetOffset(),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset()));
            break;
        case SendPropType::DPT_Int:
            logging::Info(
                XORSTR("%s [0x%04x] = %i | %u | %hd | %hu"), prop->GetName(),
                prop->GetOffset(), CE_INT(ent, acc_offset + prop->GetOffset()),
                CE_VAR(ent, acc_offset + prop->GetOffset(), unsigned int),
                CE_VAR(ent, acc_offset + prop->GetOffset(), short),
                CE_VAR(ent, acc_offset + prop->GetOffset(), unsigned short));
            break;
        case SendPropType::DPT_String:
            logging::Info(XORSTR("%s [0x%04x] = %s"), prop->GetName(),
                          prop->GetOffset(),
                          CE_VAR(ent, prop->GetOffset(), char *));
            break;
        case SendPropType::DPT_Vector:
            logging::Info(XORSTR("%s [0x%04x] = (%f, %f, %f)"), prop->GetName(),
                          prop->GetOffset(),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset()),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset() + 4),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset() + 8));
            break;
        case SendPropType::DPT_VectorXY:
            logging::Info(XORSTR("%s [0x%04x] = (%f, %f)"), prop->GetName(),
                          prop->GetOffset(),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset()),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset() + 4));
            break;
        }
    }
    if (!ft || !strcmp(ft, table->GetName()))
        logging::Info(XORSTR("==== END OF TABLE: %s"), table->GetName());
}

// CatCommand to dumb netvar info
static CatCommand
    dump_vars(XORSTR("debug_dump_netvars"), XORSTR("Dump netvars of entity"),
              [](const CCommand &args) {
                  if (args.ArgC() < 1)
                      return;
                  if (!atoi(args[1]))
                      return;
                  int idx           = atoi(args[1]);
                  CachedEntity *ent = ENTITY(idx);
                  if (CE_BAD(ent))
                      return;
                  ClientClass *clz = RAW_ENT(ent)->GetClientClass();
                  logging::Info(XORSTR("Entity %i: %s"), ent->m_IDX, clz->GetName());
                  const char *ft = (args.ArgC() > 1 ? args[2] : 0);
                  DumpRecvTable(ent, clz->m_pRecvTable, 0, ft, 0);
              });
} // namespace misc
} // namespace shared
} // namespace hacks

/*void DumpRecvTable(CachedEntity* ent, RecvTable* table, int depth, const char*
ft, unsigned acc_offset) { bool forcetable = ft && strlen(ft); if (!forcetable
|| !strcmp(ft, table->GetName())) logging::Info(XORSTR("==== TABLE: %s"),
table->GetName()); for (int i = 0; i < table->GetNumProps(); i++) { RecvProp*
prop = table->GetProp(i); if (!prop) continue; if (prop->GetDataTable()) {
            DumpRecvTable(ent, prop->GetDataTable(), depth + 1, ft, acc_offset +
prop->GetOffset());
        }
        if (forcetable && strcmp(ft, table->GetName())) continue;
        switch (prop->GetType()) {
        case SendPropType::DPT_Float:
            logging::Info(XORSTR("%s [0x%04x] = %f"), prop->GetName(),
prop->GetOffset(), CE_FLOAT(ent, acc_offset + prop->GetOffset())); break; case
SendPropType::DPT_Int: logging::Info(XORSTR("%s [0x%04x] = %i | %u | %hd | %hu"),
prop->GetName(), prop->GetOffset(), CE_INT(ent, acc_offset + prop->GetOffset()),
CE_VAR(ent, acc_offset +  prop->GetOffset(), unsigned int), CE_VAR(ent,
acc_offset + prop->GetOffset(), short), CE_VAR(ent, acc_offset +
prop->GetOffset(), unsigned short)); break; case SendPropType::DPT_String:
            logging::Info(XORSTR("%s [0x%04x] = %s"), prop->GetName(),
prop->GetOffset(), CE_VAR(ent, prop->GetOffset(), char*)); break; case
SendPropType::DPT_Vector: logging::Info(XORSTR("%s [0x%04x] = (%f, %f, %f)"),
prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset +
prop->GetOffset()), CE_FLOAT(ent, acc_offset + prop->GetOffset() + 4),
CE_FLOAT(ent, acc_offset + prop->GetOffset() + 8)); break; case
SendPropType::DPT_VectorXY: logging::Info(XORSTR("%s [0x%04x] = (%f, %f)"),
prop->GetName(), prop->GetOffset(), CE_FLOAT(ent, acc_offset +
prop->GetOffset()), CE_FLOAT(ent,acc_offset +  prop->GetOffset() + 4)); break;
        }

    }
    if (!ft || !strcmp(ft, table->GetName()))
        logging::Info(XORSTR("==== END OF TABLE: %s"), table->GetName());
}

void CC_DumpVars(const CCommand& args) {
    if (args.ArgC() < 1) return;
    if (!atoi(args[1])) return;
    int idx = atoi(args[1]);
    CachedEntity* ent = ENTITY(idx);
    if (CE_BAD(ent)) return;
    ClientClass* clz = RAW_ENT(ent)->GetClientClass();
    logging::Info(XORSTR("Entity %i: %s"), ent->m_IDX, clz->GetName());
    const char* ft = (args.ArgC() > 1 ? args[2] : 0);
    DumpRecvTable(ent, clz->m_pRecvTable, 0, ft, 0);
}*/
