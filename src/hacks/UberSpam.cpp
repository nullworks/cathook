/*
 * UberSpam.cpp
 *
 *  Created on: May 3, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <hacks/UberSpam.hpp>

namespace hacks
{
namespace tf
{
namespace uberspam
{

TextFile custom_lines;

static CatEnum source_enum({ XORSTR("DISABLED"), XORSTR("CATHOOK"), XORSTR("NCC"), XORSTR("CUSTOM") });
static CatVar source(source_enum, XORSTR("uberspam"), XORSTR("0"), XORSTR("Ubercharge Spam"),
                     XORSTR("Defines spam ubercharge source"));
static CatVar on_ready(CV_SWITCH, XORSTR("uberspam_ready"), XORSTR("1"), XORSTR("Uber Ready"));
static CatVar on_used(CV_SWITCH, XORSTR("uberspam_used"), XORSTR("1"), XORSTR("Uber Used"));
static CatVar on_ended(CV_SWITCH, XORSTR("uberspam_ended"), XORSTR("1"), XORSTR("Uber Ended"));
static CatVar on_build(CV_INT, XORSTR("uberspam_build"), XORSTR("25"), XORSTR("Uber Build"),
                       XORSTR("Send a message every #% ubercharge. 0 = never send"), 0,
                       100);
static CatVar team_chat(CV_SWITCH, XORSTR("uberspam_team"), XORSTR("1"), XORSTR("Uber Team Chat"),
                        XORSTR("Send uberspam messages in team chat"));
static CatVar
    custom_file(CV_STRING, XORSTR("uberspam_file"), XORSTR("uberspam.txt"),
                XORSTR("Ubercharge Spam File"),
                XORSTR("Use cat_uberspam_file_reload! Same as spam/killsay files."));
static CatCommand custom_file_reload(
    XORSTR("uberspam_file_reload"), XORSTR("Reload Ubercharge Spam File"),
    []() { custom_lines.Load(std::string(custom_file.GetString())); });

const std::vector<std::string> *GetSource()
{
    switch ((int) source)
    {
    case 1:
        return &builtin_cathook;
    case 2:
        return &builtin_nonecore;
    case 3:
        return &custom_lines.lines;
    }
    return nullptr;
}

int ChargePercentLineIndex(float chargef)
{
    if ((int) on_build <= 0)
        return 0;
    int charge = chargef * 100.0f;
    if (charge == 100)
        return 0;
    auto src = GetSource();
    if (!src)
        return 0;
    int lines = src->size() - 3;
    if (lines <= 0)
        return 0;
    int cpl = 100 / lines;
    return 3 + (charge / cpl);
}

void CreateMove()
{
    if (!GetSource())
        return;
    if (LOCAL_W->m_iClassID() != CL_CLASS(CWeaponMedigun))
        return;
    if (GetSource()->size() < 4)
        return;
    float charge                   = CE_FLOAT(LOCAL_W, netvar.m_flChargeLevel);
    static bool release_last_frame = false;
    static int last_charge         = 0;
    bool release                   = CE_BYTE(LOCAL_W, netvar.bChargeRelease);
    if (release_last_frame != release)
    {
        if (release)
        {
            if (on_used)
                chat_stack::Say(GetSource()->at(1), !!team_chat);
        }
        else
        {
            if (on_ended)
                chat_stack::Say(GetSource()->at(2), !!team_chat);
        }
    }
    if (!release && ((int) (100.0f * charge) != last_charge))
    {
        if (charge == 1.0f)
        {
            if (on_ready)
                chat_stack::Say(GetSource()->at(0), !!team_chat);
        }
        else
        {
            if ((int) (charge * 100.0f) != 0 && on_build)
            {
                int chargeperline = ((int) on_build >= 100)
                                        ? (100 / (GetSource()->size() - 2))
                                        : (int) on_build;
                if (chargeperline < 1)
                    chargeperline = 1;
                if ((int) (charge * 100.0f) % chargeperline == 0)
                {
                    std::string res =
                        GetSource()->at(ChargePercentLineIndex(charge));
                    ReplaceString(res, XORSTR("%i%"),
                                  std::to_string((int) (charge * 100.0f)));
                    chat_stack::Say(res, !!team_chat);
                }
            }
        }
    }
    release_last_frame = release;
    last_charge        = (int) (100.0f * charge);
}

// Ready, Used, Ended, %...

const std::vector<std::string> builtin_cathook = {
    XORSTR("-> I am charged!"),
    XORSTR("-> Not a step back! UBERCHARGE USED!"),
    XORSTR("-> My Ubercharge comes to an end!"),
    XORSTR("-> I have a bit of ubercharge!"),
    XORSTR("-> I have half of the ubercharge!"),
    XORSTR("-> Ubercharge almost ready!")
};
const std::vector<std::string> builtin_nonecore = {
    XORSTR(">>> GET READY TO RUMBLE! <<<"), XORSTR(">>> CHEATS ACTIVATED! <<<"),
    XORSTR(">>> RUMBLE COMPLETE! <<<"), XORSTR(">>> RUMBLE IS %i%% CHARGED! <<<")
};
}
}
}
