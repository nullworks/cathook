/*
 * SendDatagram.cpp
 *
 *  Created on: May 21, 2018
 *      Author: bencat07
 */
#include "HookedMethods.hpp"
#include "Backtrack.hpp"
#include "AntiCheatBypass.hpp"

namespace hooked_methods
{
DEFINE_HOOKED_METHOD(SendDatagram, int, INetChannel *ch, bf_write *buf)
{
    if (!isHackActive() || !ch || (CE_BAD(LOCAL_E) && !hacks::tf2::antianticheat::enabled) || std::floor(*hacks::tf2::backtrack::latency) == 0)
        return original::SendDatagram(ch, buf);

    int in    = ch->m_nInSequenceNr;
    int state = ch->m_nInReliableState;
    // Do backtrack things
    hacks::tf2::backtrack::adjustPing(ch);

    int ret                = original::SendDatagram(ch, buf);
    ch->m_nInSequenceNr    = in;
    ch->m_nInReliableState = state;

    return ret;
}
} // namespace hooked_methods
