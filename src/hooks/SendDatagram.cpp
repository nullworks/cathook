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
#if !LAGBOT_MODE
    int in;
    int state;
    if (CE_GOOD(LOCAL_E)) {
        in = ch->m_nInSequenceNr;
        state = ch->m_nInReliableState;

        float latencysend =
                round((round((hacks::shared::backtrack::getLatency() - 0.5f) /
                             15.1515151515f) -
                       0.5f) *
                      15.1515151515f);
        hacks::shared::backtrack::AddLatencyToNetchan(ch, latencysend);
    }
#endif

    int ret = original::SendDatagram(ch, buf);
#if !LAGBOT_MODE
    if (CE_GOOD(LOCAL_E)) {
        ch->m_nInSequenceNr = in;
        ch->m_nInReliableState = state;
    }

#endif
    return ret;
}
} // namespace hooked_methods
