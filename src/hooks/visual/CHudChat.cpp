#include "HookedMethods.hpp"
#include "MiscTemporary.hpp"

namespace hooked_methods
{

static settings::Boolean anti_spam{ "chat.anti-spam", "false" };

DEFINE_HOOKED_METHOD(StartMessageMode, int, CHudBaseChat *_this, int mode)
{
    ignoreKeys = true;
    return original::StartMessageMode(_this, mode);
}
DEFINE_HOOKED_METHOD(StopMessageMode, void *, CHudBaseChat *_this)
{
    ignoreKeys = false;
    return original::StopMessageMode(_this);
}

// This Struct stores a bunch of information about spam
struct SpamClass
{
    bool is_blocked{ false };
    std::string string{ "" };
    Timer block_time{};
    int count{ 0 }; // Count in form of: How many times has this message been sent already?
    SpamClass(std::string str)
    {
        string = str;
        block_time.update();
    }
};

// Storage for the spam messages
std::array<std::vector<SpamClass>, MAX_PLAYERS> spam_storage;
DEFINE_HOOKED_METHOD(ChatPrintf, void, CHudBaseChat *_this, int player_idx, int iFilter, const char *str, ...)
{
    auto buf = std::make_unique<char[]>(1024);
    va_list list;
    va_start(list, str);
    vsprintf(buf.get(), str, list);
    va_end(list);

    if (anti_spam)
    {
        if (player_idx > 0 && player_idx <= g_IEngine->GetMaxClients())
        {
            // Spam Entries for player
            auto &spam_vec = spam_storage.at(player_idx - 1);
            // Is it in the vector already?
            bool found_msg         = false;
            SpamClass *found_entry = nullptr;
            for (auto &i : spam_vec)
            {
                // Check if we have this entry
                if (i.string == buf.get())
                {
                    found_msg   = true;
                    found_entry = &i;
                    break;
                }
            }
            // Didn't find message, add it
            if (!found_msg)
            {
                spam_vec.push_back(SpamClass(buf.get()));
                if (spam_vec.size() > 10)
                    spam_vec.erase(spam_vec.begin());
                found_entry = &spam_vec.at(spam_vec.size() - 1);
            }
            // We Have the entry process it

            // Increment message count
            found_entry->count++;
            // Reset Block
            if (found_entry->is_blocked && found_entry->block_time.check(10000))
            {
                found_entry->is_blocked = false;
                found_entry->count      = 0;
            }
            // Update Time
            found_entry->block_time.update();
            // Stop spamming tf
            if (found_entry->count > 5)
            {
                found_entry->is_blocked = true;
                // Filter out the message
                return;
            }
        }
    }
    if (*clean_chat && isHackActive())
    {
        std::string result = buf.get();

        ReplaceString(result, "\n", "");
        ReplaceString(result, "\r", "");
        return original::ChatPrintf(_this, player_idx, iFilter, "%s", result.c_str());
    }
    return original::ChatPrintf(_this, player_idx, iFilter, "%s", buf.get());
}
static InitRoutine initlevlinit(
    []()
    {
        EC::Register(
            EC::LevelInit,
            []()
            {
                for (auto &i : spam_storage)
                    i.clear();
            },
            "clear_antispam");
    });
} // namespace hooked_methods
