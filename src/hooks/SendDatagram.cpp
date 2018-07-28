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
#if not LAGBOT_MODE
    int in     = ch->m_nInSequenceNr;
    auto state = ch->m_nInReliableState;

    float latencysend =
        round((round(((float) hacks::shared::backtrack::latency - 0.5f) /
                     15.1515151515f) -
               0.5f) *
              15.1515151515f);
    hacks::shared::backtrack::AddLatencyToNetchan(ch, latencysend);
#endif

    int ret = original::SendDatagram(ch, buf);
#if not LAGBOT_MODE

    ch->m_nInSequenceNr    = in;
    ch->m_nInReliableState = state;

#endif
    return ret;
}
}
