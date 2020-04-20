/*
  Created on 03.07.18.
*/

#pragma once

#include <menu/BaseMenuObject.hpp>
#include <menu/object/Text.hpp>
#include <menu/ObjectFactory.hpp>
#include <menu/object/container/Container.hpp>

namespace zerokernel
{

class LabeledObject : public Container
{
public:
    LabeledObject();

    ~LabeledObject() override = default;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    void reorderElements() override;

    //

    void setObject(std::unique_ptr<BaseMenuObject> &&object);

    void setLabel(std::string text);

    void createLabel();

    Text *label{ nullptr };
};
} // namespace zerokernel