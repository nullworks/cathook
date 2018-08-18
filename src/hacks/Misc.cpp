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
#include <glez/draw.hpp>
#include <settings/Bool.hpp>

#include "core/sharedobj.hpp"

#include "hack.hpp"
#include "common.hpp"

static settings::Bool render_zoomed{ "visuals.render-local-zoomed", "false" };
static settings::Bool anti_afk{ "misc.anti-afk", "false" };
static settings::Bool auto_strafe{ "misc.autostrafe", "false" };
static settings::Bool tauntslide{ "misc.tauntslide-tf2c", "false" };
static settings::Bool tauntslide_tf2{ "misc.tauntslide", "false" };
static settings::Bool flashlight_spam{ "misc.flashlight-spam", "false" };
static settings::Bool auto_balance_spam{ "misc.auto-balance-spam", "false" };
static settings::Bool nopush_enabled{ "misc.no-push", "false" };

#if ENABLE_VISUALS
static settings::Bool god_mode{ "misc.god-mode", "false" };
static settings::Bool debug_info{ "misc.debug-info", "false" };
static settings::Bool no_homo{ "misc.no-homo", "true" };
static settings::Bool show_spectators{ "misc.show-spectators", "false" };
#endif

static void *C_TFPlayer__ShouldDraw_original = nullptr;

static bool C_TFPlayer__ShouldDraw_hook(IClientEntity *thisptr)
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

static void tryPatchLocalPlayerShouldDraw()
{
    // Patching local player
    void **vtable = *(void ***) (g_pLocalPlayer->entity->InternalEntity());
    if (vtable[offsets::ShouldDraw()] != C_TFPlayer__ShouldDraw_hook)
    {
        C_TFPlayer__ShouldDraw_original = vtable[offsets::ShouldDraw()];
        void *page = (void *) ((uintptr_t) vtable & ~0xFFF);
        mprotect(page, 0xFFF, PROT_READ | PROT_WRITE | PROT_EXEC);
        vtable[offsets::ShouldDraw()] = (void *) C_TFPlayer__ShouldDraw_hook;
        mprotect(page, 0xFFF, PROT_READ | PROT_EXEC);
    }
}

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
        if (anti_afk_timer.check(60000))
        {
            // Send random commands
            current_user_cmd->sidemove    = RandFloatRange(-450.0, 450.0);
            current_user_cmd->forwardmove = RandFloatRange(-450.0, 450.0);
            current_user_cmd->buttons     = rand();
            // Prevent attack command
            current_user_cmd->buttons &= ~IN_ATTACK;
            if (anti_afk_timer.check(61000))
            {
                anti_afk_timer.update();
            }
        }
    }
}

namespace hacks::shared::misc
{

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
CatCommand
    SendAutoBlRqCatCom("request_balance", "Request Infinite Auto-Balance",
                       [](const CCommand &args) { SendAutoBalanceRequest(); });

static int last_number{ 0 };
static int last_checked_command_number{ 0 };
static IClientEntity *last_checked_weapon{ nullptr };
static bool flash_light_spam_switch{ false };
static Timer auto_balance_timer{};

static ConVar *teammatesPushaway{ nullptr };
InitRoutine init([]() {
    teammatesPushaway = g_ICvar->FindVar("tf_avoidteammates_pushaway");
});

void CreateMove()
{
#if !LAGBOT_MODE
    if (current_user_cmd->command_number)
        last_number = current_user_cmd->command_number;

    // TODO FIXME this should be moved out of here
    IF_GAME(IsTF2())
    {
        if (render_zoomed && CE_GOOD(LOCAL_E))
            tryPatchLocalPlayerShouldDraw();
    }
    // AntiAfk That after a certian time without movement keys depressed, causes
    // random keys to be spammed for 1 second
    if (anti_afk)
        updateAntiAfk();

    // Automaticly airstrafes in the air
    if (auto_strafe)
    {
        auto ground = (bool) (CE_INT(g_pLocalPlayer->entity, netvar.iFlags) &
                              FL_ONGROUND);
        if (!ground)
        {
            if (current_user_cmd->mousedx)
            {
                current_user_cmd->sidemove =
                    current_user_cmd->mousedx > 1 ? 450.f : -450.f;
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
        if (*nopush_enabled == teammatesPushaway->GetBool())
            teammatesPushaway->SetValue(!nopush_enabled);
    }
#endif
}

#if ENABLE_VISUALS

void DrawText()
{
    if (god_mode)
        for (int i = 0; i < 40000; i++)
        {
            g_ISurface->PlaySound("vo/demoman_cloakedspy03.mp3");
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
            glez::draw::rect(0, step * (i - 1), width,
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
                auto observermode = "N/A";
                switch (CE_INT(ent, netvar.iObserverMode))
                {
                case 4:
                    observermode = "Firstperson";
                    break;
                case 5:
                    observermode = "Thirdperson";
                    break;
                case 7:
                    observermode = "Freecam";
                    break;
                }
                AddSideString(format(info.name, " ", observermode));
            }
        }
    }
    if (!debug_info)
        return;
    if (CE_GOOD(g_pLocalPlayer->weapon()))
    {
        AddSideString(format("Slot: ", re::C_BaseCombatWeapon::GetSlot(
                                           RAW_ENT(g_pLocalPlayer->weapon()))));
        AddSideString(
            format("Taunt Concept: ", CE_INT(LOCAL_E, netvar.m_iTauntConcept)));
        AddSideString(
            format("Taunt Index: ", CE_INT(LOCAL_E, netvar.m_iTauntIndex)));
        AddSideString(
            format("Sequence: ", CE_INT(LOCAL_E, netvar.m_nSequence)));
        AddSideString(format("Velocity: ", LOCAL_E->m_vecVelocity.x, ' ',
                             LOCAL_E->m_vecVelocity.y, ' ',
                             LOCAL_E->m_vecVelocity.z));
        AddSideString(format("Velocity3: ", LOCAL_E->m_vecVelocity.Length()));
        AddSideString(format("Velocity2: ", LOCAL_E->m_vecVelocity.Length2D()));
        AddSideString("NetVar Velocity");
        Vector vel = CE_VECTOR(LOCAL_E, netvar.vVelocity);
        AddSideString(format("Velocity: ", vel.x, ' ', vel.y, ' ', vel.z));
        AddSideString(format("Velocity3: ", vel.Length()));
        AddSideString(format("Velocity2: ", vel.Length2D()));
        AddSideString(format("flSimTime: ",
                             LOCAL_E->var<float>(netvar.m_flSimulationTime)));
        if (current_user_cmd)
            AddSideString(format("command_number: ", last_cmd_number));
        AddSideString(format(
            "clip: ", CE_INT(g_pLocalPlayer->weapon(), netvar.m_iClip1)));
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
        for (int i = 0; i < HIGHEST_ENTITY; i++) {
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
        //AddCenterString(draw::font_handle,
        input->GetAnalogValue(AnalogCode_t::MOUSE_X),
        input->GetAnalogValue(AnalogCode_t::MOUSE_Y), draw::white,
        L"S\u0FD5");*/
    }
}

#endif

void Schema_Reload()
{
    logging::Info("Custom schema loading is not supported right now.");

    static uintptr_t InitSchema_s = gSignatures.GetClientSignature(
        "55 89 E5 57 56 53 83 EC ? 8B 5D ? 8B 7D ? 8B 03 89 1C 24 FF 50 ? C7 "
        "04 24 ? ? ? ?");
    typedef bool (*InitSchema_t)(void *, CUtlBuffer &, int);
    static InitSchema_t InitSchema   = (InitSchema_t) InitSchema_s;
    static uintptr_t GetItemSchema_s = gSignatures.GetClientSignature(
        "55 89 E5 83 EC ? E8 ? ? ? ? C9 83 C0 ? C3 55 89 E5 8B 45 ?");
    typedef void *(*GetItemSchema_t)(void);
    static GetItemSchema_t GetItemSchema = (GetItemSchema_t)
        GetItemSchema_s; //(*(uintptr_t*)GetItemSchema_s +GetItemSchema_s + 4);

    logging::Info("0x%08x 0x%08x", InitSchema, GetItemSchema);
    void *itemschema = (void *) ((unsigned) GetItemSchema() + 4);
    void *data;
    char *path = strfmt("/opt/cathook/data/items_game.txt").get();
    FILE *file = fopen(path, "r");
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
        logging::Info("Error loading file");
        fclose(file);
        return;
    }
    fclose(file);
    logging::Info("0x%08x 0x%08x", InitSchema, GetItemSchema);
    bool ret = InitSchema(GetItemSchema(), buf, 133769);
    logging::Info("Loading %s", ret ? "Successful" : "Unsuccessful");
}
CatCommand schema("schema", "Load custom schema", Schema_Reload);

CatCommand name("name_set", "Immediate name change", [](const CCommand &args) {
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
    ReplaceString(new_name, "\\n", "\n");
    NET_SetConVar setname("name", new_name.c_str());
    INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    if (ch)
    {
        setname.SetNetChannel(ch);
        setname.SetReliable(false);
        ch->SendNetMsg(setname, false);
    }
});
CatCommand set_value("set", "Set value", [](const CCommand &args) {
    if (args.ArgC() < 2)
        return;
    ConVar *var = g_ICvar->FindVar(args.Arg(1));
    if (!var)
        return;
    std::string value(args.Arg(2));
    ReplaceString(value, "\\n", "\n");
    var->SetValue(value.c_str());
    logging::Info("Set '%s' to '%s'", args.Arg(1), value.c_str());
});
CatCommand say_lines("say_lines", "Say with newlines (\\n)",
                     [](const CCommand &args) {
                         std::string message(args.ArgS());
                         ReplaceString(message, "\\n", "\n");
                         std::string cmd = format("say ", message);
                         g_IEngine->ServerCmd(cmd.c_str());
                     });
CatCommand disconnect("disconnect", "Disconnect with custom reason",
                      [](const CCommand &args) {
                          INetChannel *ch =
                              (INetChannel *) g_IEngine->GetNetChannelInfo();
                          if (!ch)
                              return;
                          ch->Shutdown(args.ArgS());
                      });

CatCommand disconnect_vac("disconnect_vac", "Disconnect (fake VAC)", []() {
    INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    if (!ch)
        return;
    ch->Shutdown("VAC banned from secure server\n");
});

// Netvars stuff
void DumpRecvTable(CachedEntity *ent, RecvTable *table, int depth,
                   const char *ft, unsigned acc_offset)
{
    bool forcetable = ft && strlen(ft);
    if (!forcetable || !strcmp(ft, table->GetName()))
        logging::Info("==== TABLE: %s", table->GetName());
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
            logging::Info("%s [0x%04x] = %f", prop->GetName(),
                          prop->GetOffset(),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset()));
            break;
        case SendPropType::DPT_Int:
            logging::Info(
                "%s [0x%04x] = %i | %u | %hd | %hu", prop->GetName(),
                prop->GetOffset(), CE_INT(ent, acc_offset + prop->GetOffset()),
                CE_VAR(ent, acc_offset + prop->GetOffset(), unsigned int),
                CE_VAR(ent, acc_offset + prop->GetOffset(), short),
                CE_VAR(ent, acc_offset + prop->GetOffset(), unsigned short));
            break;
        case SendPropType::DPT_String:
            logging::Info("%s [0x%04x] = %s", prop->GetName(),
                          prop->GetOffset(),
                          CE_VAR(ent, prop->GetOffset(), char *));
            break;
        case SendPropType::DPT_Vector:
            logging::Info("%s [0x%04x] = (%f, %f, %f)", prop->GetName(),
                          prop->GetOffset(),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset()),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset() + 4),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset() + 8));
            break;
        case SendPropType::DPT_VectorXY:
            logging::Info("%s [0x%04x] = (%f, %f)", prop->GetName(),
                          prop->GetOffset(),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset()),
                          CE_FLOAT(ent, acc_offset + prop->GetOffset() + 4));
            break;
        }
    }
    if (!ft || !strcmp(ft, table->GetName()))
        logging::Info("==== END OF TABLE: %s", table->GetName());
}

// CatCommand to dumb netvar info
static CatCommand
    dump_vars("debug_dump_netvars", "Dump netvars of entity",
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
                  logging::Info("Entity %i: %s", ent->m_IDX, clz->GetName());
                  const char *ft = (args.ArgC() > 1 ? args[2] : 0);
                  DumpRecvTable(ent, clz->m_pRecvTable, 0, ft, 0);
              });
} // namespace hacks::shared::misc

/*void DumpRecvTable(CachedEntity* ent, RecvTable* table, int depth, const char*
ft, unsigned acc_offset) { bool forcetable = ft && strlen(ft); if (!forcetable
|| !strcmp(ft, table->GetName())) logging::Info("==== TABLE: %s",
table->GetName()); for (int i = 0; i < table->GetNumProps(); i++) { RecvProp*
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
