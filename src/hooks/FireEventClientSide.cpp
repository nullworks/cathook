/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "HookedMethods.hpp"

namespace hacks::killstreak
{
extern void fire_event(IGameEvent *event);
}
namespace hooked_methods
{

#if !ENFORCE_STREAM_SAFETY
static settings::Boolean enable_antispam{ "chat.party.anticrash.enabled", "true" };
#else
static settings::Boolean enable_antispam{ "chat.party.anticrash.enabled", "false" };
#endif

// max messages in 5 seconds before being flagged
static settings::Int spam_rate{ "chat.party.anticrash.limit-rate", "12" };

static int spam_messages_since_reset{ 0 };
static Timer spam_timer{};
static bool spam_blocked = true;

DEFINE_HOOKED_METHOD(FireEventClientSide, bool, IGameEventManager2 *this_, IGameEvent *event)
{
    if (enable_antispam && strcmp(event->GetName(), "party_chat") == 0)
    {
        // Increase total count
        spam_messages_since_reset++;
        // Limit reached
        if (spam_messages_since_reset > *spam_rate)
        {
            spam_blocked = true;
            spam_timer.update();
            spam_messages_since_reset = 0;
        }

        if (spam_blocked)
        {
            // Check if we can lift the ban again
            if (spam_timer.test_and_set(10000))
            {
                spam_messages_since_reset = 0;
                // Need to readd 1 to the timer since the current message counts towards the next "epoch"
                spam_messages_since_reset++;
                spam_blocked = false;
            }
            else
                // Void messages to avoid crash
                return true;
        }
        // We havent reached the limit yet. Reset the limit if last reset is 5 seconds in the past
        else if (spam_timer.test_and_set(5000))
        {
            spam_messages_since_reset = 0;
            // Need to readd 1 to the timer since the current message counts towards the next "epoch"
            spam_messages_since_reset++;
        }
    }
    hacks::killstreak::fire_event(event);
    return original::FireEventClientSide(this_, event);
}
} // namespace hooked_methods
