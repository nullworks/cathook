/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "HookedMethods.hpp"

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(GetUserCmd, CUserCmd *, IInput *this_, int sequence_number)
{
    CUserCmd *def = original::GetUserCmd(this_, sequence_number);
    int oldcmd;
    INetChannel *ch;

    if (def == nullptr)
        return def;

    if (command_number_mod.find(def->command_number) !=
        command_number_mod.end())
    {
        // logging::Info("Replacing command %i with %i", def->command_number,
        // command_number_mod[def->command_number]);
        oldcmd              = def->command_number;
        def->command_number = command_number_mod[def->command_number];
        def->random_seed =
            MD5_PseudoRandom(unsigned(def->command_number)) & 0x7fffffff;
        command_number_mod.erase(command_number_mod.find(oldcmd));
        *(int *) ((unsigned) g_IBaseClientState +
                  offsets::lastoutgoingcommand()) = def->command_number - 1;
        ch =
            (INetChannel *) g_IEngine
                ->GetNetChannelInfo(); //*(INetChannel**)((unsigned)g_IBaseClientState
        //+ offsets::m_NetChannel());
        *(int *) ((unsigned) ch + offsets::m_nOutSequenceNr()) =
            def->command_number - 1;
    }
    return def;
}
} // namespace hooked_methods