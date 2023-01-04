/*
 * mchealthbar.cpp
 *
 *  Created on: Apr 08, 2019
 *      Author: UNKN0WN
 */

#include "common.hpp"

namespace mchealthbar
{
// qtcreator marks no extern declared but there is, so don't make it static thanks
settings::Boolean minecraftHP("mc_health.enable", "false");
static settings::Float size("mc_health.size", "32");
static std::vector<textures::sprite> absorption;
static std::vector<textures::sprite> hearts;

static InitRoutine init_textures([]() {
    for (int i = 0; i < 2; ++i)
        absorption.push_back(textures::atlas().create_sprite(320 + 64 * i, 64, 64, 64));
    for (int i = 0; i < 3; ++i)
        hearts.push_back(textures::atlas().create_sprite(256 + 64 * i, 192, 64, 64));
});

void draw_func()
{
    if (!minecraftHP || CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W))
        return;
    float iconSize  = *size;
    float startPosX = draw::width * 0.2f - iconSize * 5.0f;
    float startPosY = draw::height - iconSize - 1.0f;

    float halfHeart  = LOCAL_E->m_iMaxHealth() / 20;     // HP per half heart
    float halfHearts = LOCAL_E->m_iHealth() / halfHeart; // health in half hearts
    int fullHearts   = (halfHearts / 20) * 10;           // HP in full hearts

    float maxBuffHealth       = floorf((g_pPlayerResource->GetMaxHealth(LOCAL_E) * 1.5f) / 10.0f) * 10.0f - 10.0f; // max buffed Health
    float maxAdditionalHealth = maxBuffHealth - LOCAL_E->m_iMaxHealth();                                           // How much additional health can one have at max
    float absorption_health   = LOCAL_E->m_iHealth() - LOCAL_E->m_iMaxHealth();                                    // Extra health
    int absorb_halfh          = ((float) absorption_health / (float) maxAdditionalHealth) * 20.0f;
    int absorb_fullh          = (absorb_halfh - 1) / 2;
    float iconPixel           = (iconSize / 8);
    for (int i = 0; i < 10; ++i)
    {
        // (iconSize / 8) = 1 pixel in the icon

        if (i + 1 <= fullHearts)
            hearts[2].draw(startPosX + (iconSize - iconPixel) * i, startPosY, iconSize, iconSize,
                           colors::white); // full heart
        else if (fullHearts + 1 == i + 1 && !((int) halfHearts % 2 == 0))
            hearts[1].draw(startPosX + (iconSize - iconPixel) * i, startPosY, iconSize, iconSize,
                           colors::white); // last half heart
        else
            hearts[0].draw(startPosX + (iconSize - iconPixel) * i, startPosY, iconSize, iconSize,
                           colors::white); // empty
    }
    if (absorb_halfh >= 1.0f)
        for (int i = 0; i < 10; ++i)
        {

            if (absorb_fullh >= i + 1)
                absorption[1].draw(startPosX + (iconSize - iconPixel) * i, startPosY, iconSize, iconSize, colors::white);
            else if ((float) absorb_halfh / 2.0f >= i + 1)
                absorption[0].draw(startPosX + (iconSize - iconPixel) * i, startPosY, iconSize, iconSize, colors::white);
        }
}
static InitRoutine routint([]() { EC::Register(EC::Draw, draw_func, "MINECRAFT-HEALTHBAR"); });
} // namespace mchealthbar
