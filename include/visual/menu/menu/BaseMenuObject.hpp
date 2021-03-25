/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <SDL2/SDL_events.h>
#include <vector>
#include <menu/interface/IMessageHandler.hpp>
#include <functional>
#include <menu/tinyxml2.hpp>
#include <menu/KeyValue.hpp>
#include <drawing.hpp>
#include "BoundingBox.hpp"

namespace zerokernel
{

class Menu;
class BoundingBox;

class BaseMenuObject : public IMessageHandler
{
public:
    friend class ModalBehavior;

    static std::size_t objectCount;

    inline virtual ~BaseMenuObject()
    {
        --objectCount;
        // printf("~BaseMenuObject %u\n", --objectCount);
    }

    inline BaseMenuObject() : uid(object_sequence_number), kv("Object" + std::to_string(uid)), bb{ *this }
    {
        ++object_sequence_number;
        // printf("BaseMenuObject(%u) %u\n", uid, ++objectCount);
    }

    virtual bool handleSdlEvent(SDL_Event *event);

    virtual void render();

    virtual void update();

    virtual void setParent(BaseMenuObject *parent);

    virtual void loadFromXml(const tinyxml2::XMLElement *data);

    virtual bool isHidden();

    virtual void hide();

    virtual void show();

    virtual BaseMenuObject *findElement(const std::function<bool(BaseMenuObject *)> &search);

    virtual void updateIsHovered();

    virtual void onMove();

    virtual void onChildMove();

    virtual void onParentMove();

    void handleMessage(Message &msg, bool is_relayed) override;

    // End of virtual methods

    /*
     * Explicitly moves the object - updates location and calls childMove on
     * children.
     */
    void move(int x, int y);

    void updateLocation();

    bool isHovered();
    
    bool isHoveredText(int text_length);

    fonts::font *font{ nullptr };

    void addMessageHandler(IMessageHandler &handler);

    bool containsMouse();

    void markForDelete();

    BaseMenuObject *getElementById(const std::string &id);

    /*
     * Event handling (WIP)
     */
    virtual bool onLeftMouseClick();

    /*
     * Size
     */
    virtual void onParentSizeUpdate();
    virtual void onChildSizeUpdate();
    virtual void emitSizeUpdate();
    virtual void recalculateSize();
    virtual void recursiveSizeUpdate();

    BoundingBox &getBoundingBox();

    void resize(int width, int height);

    /*
     * Rendering helper methods
     */

    virtual void renderDebugOverlay();
    void renderBackground(rgba_t color);
    void renderBorder(rgba_t color);

    int xOffset{ 0 };
    int yOffset{ 0 };

    bool markedForDelete{ false };
    BaseMenuObject *parent{ nullptr };

    bool hidden{ false };

    std::optional<std::string> tooltip{};
    
    std::string label{};

    size_t hovered_frame{ 0 };

    std::string string_id{};
    const size_t uid;
    KeyValue kv;
    BoundingBox bb;

protected:
    void emit(Message &msg, bool is_relayed);

    std::vector<IMessageHandler *> handlers{};

    static size_t object_sequence_number;
};
} // namespace zerokernel
