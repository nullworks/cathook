/*
  Created on 07.07.18.
*/

#include <menu/object/container/LabeledObject.hpp>

void zerokernel::LabeledObject::loadFromXml(const tinyxml2::XMLElement *data)
{
    Container::loadFromXml(data);

    const char *label_text;
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("label", &label_text))
    {
        setLabel(label_text);
    }
}

zerokernel::LabeledObject::LabeledObject() : BaseMenuObject{}
{
    bb.width.setContent();
    bb.height.setContent();
}

void zerokernel::LabeledObject::createLabel()
{
    auto label = std::make_unique<Text>();
    label->setParent(this);
    this->label = label.get();
    label->bb.setMargin(0, 0, 6, 0);
    label->bb.width.setContent();
    label->bb.height.setFill();
    addObject(std::move(label));
}

void zerokernel::LabeledObject::setLabel(std::string text)
{
    if (label == nullptr)
    {
        createLabel();
    }
    label->set(std::move(text));
}

void zerokernel::LabeledObject::reorderElements()
{
    if (!objects.empty())
    {
        int x = bb.getContentBox().width - objects[0]->getBoundingBox().getFullBox().width;
        if (this->label)
        {
            if (x < this->label->getBoundingBox().getFullBox().width + this->label->getBoundingBox().margin.right + objects[0]->getBoundingBox().margin.left)
                x = this->label->getBoundingBox().getFullBox().width + this->label->getBoundingBox().margin.right + objects[0]->getBoundingBox().margin.left;
        }
        objects[0]->move(x, 0);
    }
    if (objects.size() > 1)
    {
        objects[1]->move(0, 0);
    }
}

void zerokernel::LabeledObject::setObject(std::unique_ptr<zerokernel::BaseMenuObject> &&object)
{
    addObject(std::move(object));
}
