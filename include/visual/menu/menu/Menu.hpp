/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <SDL2/SDL_events.h>
#include <settings/Registered.hpp>
#include <menu/wm/WindowManager.hpp>
#include <menu/object/container/Container.hpp>
#include "Tooltip.hpp"
#include "BoundingBox.hpp"

namespace zerokernel
{

namespace resource::font
{
extern fonts::font base;
extern fonts::font bold;
} // namespace resource::font

namespace style::colors
{
using color_type = settings::RVariable<rgba_t>;

extern color_type text;
extern color_type text_shadow;
extern color_type error;
} // namespace style::colors

class Menu : public IMessageHandler
{
public:
    Menu(int w, int h);

    static Menu *instance;

    static void init(int w, int h);
    static void destroy();

    bool handleSdlEvent(SDL_Event *event);
    void render();
    void update();

    void addModalObject(std::unique_ptr<BaseMenuObject> &&object);

    void setup();
    void reset();

    void loadFromFile(std::string directory, std::string path);
    void loadFromXml(tinyxml2::XMLElement *element);

    void handleMessage(Message &msg, bool is_relayed) override;

    void showTooltip(std::string tooltip);
    void updateHovered();

    bool isInGame();

    void setInGame(bool flag);

    void resize(int x, int y);

    BoundingBox &wmRootBoundingBox();

    tinyxml2::XMLElement *getPrefab(const std::string &name);

    int mouseX;
    int mouseY;

    int screen_x{};
    int screen_y{};

    int dx;
    int dy;

    bool ready{ false };
    int modal_close_next_frame{ 0 };
    size_t frame{ 0 };

    bool in_game{ false };

    std::unique_ptr<WindowManager> wm{ nullptr };
    std::vector<std::unique_ptr<BaseMenuObject>> modal_stack{};
    Tooltip tooltip{};
    tinyxml2::XMLDocument xml_source{};
    boost::unordered_flat_map<std::string, tinyxml2::XMLElement *> prefabs{};
};
} // namespace zerokernel
