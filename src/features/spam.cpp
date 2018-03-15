
/*
 *
 * Has stuff to do with spamming, be it in chat, or with other methods of annoyance!
 *
 *
 */

#include <vector>

#include "../util/chrono.hpp"
#include "../util/catvars.hpp"
#include "../util/iohelper.hpp"
#include "../util/logging.hpp"
#include "../framework/gameticks.hpp"

#include "spam.hpp"

namespace features::spam {

const static CatEnum spam_menu({"Spam"});
static CatEnum spam_type_enum({"OFF", "NEKOHOOK", "CATHOOK", "LMAOBOX", "LITHIUM", "NULLCORE", "CUSTOM"});
static CatVarEnum spam_type(spam_menu, spam_type_enum, "spam", 0, "Spam", "Choose your type of spam!");
static CatVarBool spam_random(spam_menu, "spam_random", true, "Spam Random", "Randomly use a line to spam!");
static CatVarString spam_file(spam_menu, "spam_file", "default.txt", "Spam File", "Put in a file to use with custom spam");
static CatVarFloat spam_time(spam_menu, "spam_time", 2, "Spam Time", "Time to wait between each spam!");

const static std::vector<std::string> nekohook_spam({
  // Onee-chan's
  "Nekohook - Did I mention that I run arch!",
  "Nekohook - Only real gamers use linux!",
  "Nekohook - the only true multi-hack!",
  "Nekohook - supporting over 2 games!", // I need more games :/
  "Nekohook - Get it at https://github.com/oneechanhax/nekohook", // Why not advertise while were at it!
  "Nekohook - Get it at https://github.com/oneechanhax/nekohook",
  // Credits to Nopey Nopey
  "Nekohook - Minecraft W10 Edition Support coming soon!",
  "Nekohook - The father is not the son",
  "Nekohook - I crashed GDB. Do I get a cookie?",
  "Nekohook - my cheat works",
  "Nekohook - :thk:"

  // More?
});
const static std::vector<std::string> cathook_spam({
    "cathook - more fun than a ball of yarn!",
    "GNU/Linux is the best OS!",
    "visit youtube.com/c/nullifiedcat for more information!",
    "cathook - free tf2 cheat!",
    "cathook - ca(n)t stop me meow!"
});
const static std::vector<std::string> lmaobox_spam({
    "GET GOOD, GET LMAOBOX!",
    "LMAOBOX - WAY TO THE TOP",
    "WWW.LMAOBOX.NET - BEST FREE TF2 HACK!"
});
const static std::vector<std::string> lithium_spam({
    "CHECK OUT www.YouTube.com/c/DurRud FOR MORE INFORMATION!",
    "PWNING AIMBOTS WITH OP ANTI-AIMS SINCE 2015 - LITHIUMCHEAT",
    "STOP GETTING MAD AND STABILIZE YOUR MOOD WITH LITHIUMCHEAT!",
    "SAVE YOUR MONEY AND GET LITHIUMCHEAT! IT IS FREE!",
    "GOT ROLLED BY LITHIUM? HEY, THAT MEANS IT'S TIME TO GET LITHIUMCHEAT!!"
});
const static std::vector<std::string> nullcrap_spam({
    "NULL CORE - REDUCE YOUR RISK OF BEING OWNED!",
    "NULL CORE - WAY TO THE TOP!",
    "NULL CORE - BEST TF2 CHEAT!",
    "NULL CORE - NOW WITH BLACKJACK AND HOOKERS!",
    "NULL CORE - BUTTHURT IN 10 SECONDS FLAT!",
    "NULL CORE - WHOLE SERVER OBSERVING!",
    "NULL CORE - GET BACK TO PWNING!",
    "NULL CORE - WHEN PVP IS TOO HARDCORE!",
    "NULL CORE - CAN CAUSE KIDS TO RAGE!",
    "NULL CORE - F2P NOOBS WILL BE 100% NERFED!"
});

static std::vector<std::string> custom_spam;

// This is a cat command so you can forcibly reload it
static CatCommand spam_reload("spam_reload", [](std::vector<std::string>){
  custom_spam = io::ReadFile(io::GetSaveLocation() + "spam/" + (std::string)spam_file);
  g_CatLogging.log("Reloaded Custom Spam!");
});

static CatCommand spam_add("spam_add", [](std::vector<std::string> args){
  if (args.empty()) {
    g_CatLogging.log("Nothing to add to custom spam!");
    return;
  }
  custom_spam.push_back(args.at(0));
  g_CatLogging.log("Added \"%s\" to custom spam!");
});

static CatCommand spam_remove("spam_remove", [](std::vector<std::string> args){
  g_CatLogging.log("Feature TODO, set custom spam to something that doesnt exist and reload to clear all!");
  return;
  if (args.empty()) {
    g_CatLogging.log("Nothing to remove from custom spam!");
    return;
  }
  custom_spam.push_back(args.at(0));
  g_CatLogging.log("Removed all instances of \"%s\" from custom spam!");
});

static std::string GetSpamString(){

  // First we need to get the type of spam
  const std::vector<std::string>* spam_group = nullptr;
  switch(spam_type) {
  case 1: spam_group = &nekohook_spam; break;
  case 2: spam_group = &cathook_spam; break;
  case 3: spam_group = &lmaobox_spam; break;
  case 4: spam_group = &lithium_spam; break;
  case 5: spam_group = &nullcrap_spam; break;
  case 6: {
    spam_group = &custom_spam;
    // Here we make sure the custom spam is updated and that we have something
    static std::string last_string = spam_file;
    if (last_string != (std::string)spam_file || custom_spam.empty()) {
      spam_reload({});
      last_string = spam_file;
    }
  }}

  // In case some idiot sets the spam_type var too high or low
  if (!spam_group)
    return std::string();

  // Spam number
  static size_t last_spam = 0; // last line spammed
  last_spam++;
  if (last_spam >= spam_group->size()) // clamp around
    last_spam = 0;

  return spam_group->at(spam_random ? rand() % spam_group->size() : last_spam);
}

// Externed, please set in your module to enable chat spam features
CMFunction<void(const char*)> SayChat {[](auto){}};

// TODO, thread
static void SpamLoop() {
  if (!spam_type || !spam_time) return;

  // Check if its time to spam
  static CatTimer last_spam;
  if (last_spam.ResetAfter(std::chrono::seconds(spam_time))) {

    auto spam_string = GetSpamString();
    if (spam_string.empty())
      return;

    SayChat(spam_string.c_str());
  }
}

void Init(){

  wtickmgr.REventDuring(SpamLoop);

}

}
