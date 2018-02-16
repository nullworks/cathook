/*
 * Created on: Fed 14. 2018
 * Author: LIghtyPonce
 */

#include "common.hpp"
#include "gui/menu.hpp"
#include "sdk.hpp"


int aimbot_en = 0;
int aimbot_en2 = 0;
int aimbot_en3 = 0;
int aimbot_en4 = 0;
int aimbot_en5 = 0;
int aimbot_en6 = 0;
int aimbot_en7 = 0;
int aimbot_en8 = 0;
int aimbot_en9 = 0;
int aimbot_en10 = 0;
int aimbot_en11 = 0;

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

    if (aimbot)
    {
        if (!aimbot_en)
        {
            g_pCVar->FindVar("cat_aimbot_enabled")->SetValue(1);
            aimbot_en = 1;
        }
    }
    else
    {
        if (aimbot_en)
        {
            g_pCVar->FindVar("cat_aimbot_enabled")->SetValue(0);
            aimbot_en = 0;
        }
    }
    static ConVar* aimbot_stuff = g_pCVar->FindVar("cat_aimbot_enabled");
    if(aimbot_stuff->GetBool())
    {
        aimbot = true;
    }
    else
    {
        aimbot = false;
    }
    static ConVar* aimbot_key_mode = g_pCVar->FindVar("cat_aimbot_aimkey_mode");
    int AKM = (aimbot_key_mode->GetInt());
    if (aimkey_mode != AKM)
    {
        if (aimkey_mode_num % 2)
        {
            g_pCVar->FindVar("cat_aimbot_aimkey_mode")->SetValue(aimkey_mode);
            aimkey_mode_num++;
        }
        else
        {
            aimkey_mode = AKM;
        }
    }
    if (aimbot_autoshoot)
    {
        if (!aimbot_en2)
        {
            g_pCVar->FindVar("cat_aimbot_autoshoot")->SetValue(1);
            aimbot_en2 = 1;
        }
    }
    else
    {
        if (aimbot_en2)
        {
            g_pCVar->FindVar("cat_aimbot_autoshoot")->SetValue(0);
            aimbot_en2 = 0;
        }
    }
    static ConVar* autoshoot = g_pCVar->FindVar("cat_aimbot_autoshoot");
    if(autoshoot->GetBool())
    {
        aimbot_autoshoot = true;
    }
    else
    {
        aimbot_autoshoot = false;
    }
    static ConVar* hitbox_mode = g_pCVar->FindVar("cat_aimbot_hitboxmode");
    int HM = (hitbox_mode->GetInt());
    if (aimbot_hitboxmode != HM)
    {
        if (aimbot_hitboxmode_num % 2)
        {
            g_pCVar->FindVar("cat_aimbot_hitboxmode")->SetValue(aimbot_hitboxmode);
            aimbot_hitboxmode_num++;
        }
        else
        {
            aimbot_hitboxmode = HM;
        }
    }
    g_pCVar->FindVar("cat_aimbot_fov")->SetValue(aimbot_fov);
    g_pCVar->FindVar("cat_aimbot_fov_draw_opacity")->SetValue(aimbot_fov_draw_opacity);
    static ConVar* priority_mode = g_pCVar->FindVar("cat_aimbot_prioritymode");
    int PM = (priority_mode->GetInt());
    if (aimbot_prioritymode != PM)
    {
        if (aimbot_prioritymode_num % 2)
        {
            g_pCVar->FindVar("cat_aimbot_prioritymode")->SetValue(aimbot_prioritymode);
            aimbot_prioritymode_num++;
        }
        else
        {
            aimkey_mode = PM;
        }
    }   
    if (aimbot_charge)
    {
        if (!aimbot_en3)
        {
            g_pCVar->FindVar("cat_aimbot_charge")->SetValue(1);
            aimbot_en3 = 1;
        }
    }
    else
    {
        if (aimbot_en3)
        {
            g_pCVar->FindVar("cat_aimbot_charge")->SetValue(0);
            aimbot_en3 = 0;
        }
    }
    static ConVar* charge = g_pCVar->FindVar("cat_aimbot_charge");
    if(charge->GetBool())
    {
        aimbot_charge = true;
    }
    else
    {
        aimbot_charge = false;
    } 
    if (aimbot_ignore_hoovy)
    {
        if (!aimbot_en4)
        {
            g_pCVar->FindVar("cat_aimbot_ignore_hoovy")->SetValue(1);
            aimbot_en4 = 1;
        }
    }
    else
    {
        if (aimbot_en4)
        {
            g_pCVar->FindVar("cat_aimbot_ignore_hoovy")->SetValue(0);
            aimbot_en4 = 0;
        }
    }
    static ConVar* ign_hoovy = g_pCVar->FindVar("cat_aimbot_ignore_hoovy");
    if(ign_hoovy->GetBool())
    {
        aimbot_ignore_hoovy = true;
    }
    else
    {
        aimbot_ignore_hoovy = false;
    } 
    if (aimbot_ignore_cloak)
    {
        if (!aimbot_en5)
        {
            g_pCVar->FindVar("cat_aimbot_ignore_cloak")->SetValue(1);
            aimbot_en5 = 1;
        }
    }
    else
    {
        if (aimbot_en5)
        {
            g_pCVar->FindVar("cat_aimbot_ignore_cloak")->SetValue(0);
            aimbot_en5 = 0;
        }
    }
    static ConVar* ign_cloak = g_pCVar->FindVar("cat_aimbot_ignore_cloak");
    if(ign_cloak->GetBool())
    {
        aimbot_ignore_cloak = true;
    }
    else
    {
        aimbot_ignore_cloak = false;
    } 
    if (aimbot_ignore_deadringer)
    {
        if (!aimbot_en6)
        {
            g_pCVar->FindVar("cat_aimbot_ignore_deadringer")->SetValue(1);
            aimbot_en6 = 1;
        }
    }
    else
    {
        if (aimbot_en6)
        {
            g_pCVar->FindVar("cat_aimbot_ignore_deadringer")->SetValue(0);
            aimbot_en6 = 0;
        }
    }
    static ConVar* ign_deadringer = g_pCVar->FindVar("cat_aimbot_ignore_deadringer");
    if(ign_deadringer->GetBool())
    {
        aimbot_ignore_deadringer = true;
    }
    else
    {
        aimbot_ignore_deadringer = false;
    } 
    if (aimbot_buildings_sentry)
    {
        if (!aimbot_en7)
        {
            g_pCVar->FindVar("cat_aimbot_buildings_sentry")->SetValue(1);
            aimbot_en7 = 1;
        }
    }
    else
    {
        if (aimbot_en7)
        {
            g_pCVar->FindVar("cat_aimbot_buildings_sentry")->SetValue(0);
            aimbot_en7 = 0;
        }
    }
    static ConVar* buildings_sent = g_pCVar->FindVar("cat_aimbot_buildings_sentry");
    if(buildings_sent->GetBool())
    {
        aimbot_buildings_sentry = true;
    }
    else
    {
        aimbot_buildings_sentry = false;
    } 
    if (aimbot_buildings_other)
    {
        if (!aimbot_en8)
        {
            g_pCVar->FindVar("cat_aimbot_buildings_other")->SetValue(1);
            aimbot_en8 = 1;
        }
    }
    else
    {
        if (aimbot_en8)
        {
            g_pCVar->FindVar("cat_aimbot_buildings_other")->SetValue(0);
            aimbot_en8 = 0;
        }
    }
    static ConVar* buildings_oth = g_pCVar->FindVar("cat_aimbot_buildings_other");
    if(buildings_oth->GetBool())
    {
        aimbot_buildings_other = true;
    }
    else
    {
        aimbot_buildings_other = false;
    } 
    if (aimbot_stickys)
    {
        if (!aimbot_en9)
        {
            g_pCVar->FindVar("cat_aimbot_stickys")->SetValue(1);
            aimbot_en9 = 1;
        }
    }
    else
    {
        if (aimbot_en9)
        {
            g_pCVar->FindVar("cat_aimbot_stickys")->SetValue(0);
            aimbot_en9 = 0;
        }
    }
    static ConVar* aimbot_stic = g_pCVar->FindVar("cat_aimbot_stickys");
    if(aimbot_stic->GetBool())
    {
        aimbot_stickys = true;
    }
    else
    {
        aimbot_stickys = false;
    } 
    if (aimbot_rage_only)
    {
        if (!aimbot_en10)
        {
            g_pCVar->FindVar("cat_aimbot_rage_only")->SetValue(1);
            aimbot_en10 = 1;
        }
    }
    else
    {
        if (aimbot_en10)
        {
            g_pCVar->FindVar("cat_aimbot_rage_only")->SetValue(0);
            aimbot_en10 = 0;
        }
    }
    static ConVar* rage_only = g_pCVar->FindVar("cat_aimbot_rage_only");
    if(rage_only->GetBool())
    {
        aimbot_rage_only = true;
    }
    else
    {
        aimbot_rage_only = false;
    } 
    static ConVar* teammates_aim = g_pCVar->FindVar("cat_aimbot_teammates");
    int TA = (teammates_aim->GetInt());
    if (aimbot_teammates != TA)
    {
        if (aimbot_teammates_num % 2)
        {
            g_pCVar->FindVar("cat_aimbot_teammates")->SetValue(aimbot_teammates);
            aimbot_teammates_num++;
        }
        else
        {
            aimbot_teammates_num = TA;
        }
    }
    if (aimbot_fov_draw)
    {
        if (!aimbot_en11)
        {
            g_pCVar->FindVar("cat_aimbot_fov_draw")->SetValue(1);
            aimbot_en11 = 1;
        }
    }
    else
    {
        if (aimbot_en11)
        {
            g_pCVar->FindVar("cat_aimbot_fov_draw")->SetValue(0);
            aimbot_en11 = 0;
        }
    }
    static ConVar* fov_draw = g_pCVar->FindVar("cat_aimbot_fov_draw");
    if(fov_draw->GetBool())
    {
        aimbot_fov_draw = true;
    }
    else
    {
        aimbot_fov_draw = false;
    } 
}
