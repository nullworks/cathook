/*
 * SendDatagram.cpp
 *
 *  Created on: May 21, 2018
 *      Author: bencat07
 */
#include "HookedMethods.hpp"
#include "hacks/Backtrack.hpp"
namespace hooked_methods
{
DEFINE_HOOKED_METHOD(SendDatagram, int, INetChannel *ch, bf_write *buf)
{
    if (!isHackActive() || !ch || CE_BAD(LOCAL_E) || !round(*hacks::shared::backtrack::latency))
        return original::SendDatagram(ch, buf);

    int in    = ch->m_nInSequenceNr;
    int state = ch->m_nInReliableState;
    hacks::shared::backtrack::AddLatencyToNetchan(ch);

    int ret                = original::SendDatagram(ch, buf);
    ch->m_nInSequenceNr    = in;
    ch->m_nInReliableState = state;

    return ret;
}
} // namespace hooked_methods
