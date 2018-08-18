#include <settings/Settings.hpp>
#include <menu/special/VariableListEntry.hpp>
#include <menu/object/input/Checkbox.hpp>
#include <menu/object/input/StringInput.hpp>
#include <menu/object/input/ColorSelector.hpp>
#include <menu/object/input/InputKey.hpp>
#include <menu/object/input/Spinner.hpp>
#include <menu/menu/special/SettingsManagerList.hpp>
#include <menu/menu/special/VariableListEntry.hpp>

static settings::RVariable<glez::rgba> marked_color{
    "zk.color.variable-list.color.registered", "ffff00"
};

/*
  Created on 26.07.18.
*/

void zerokernel::VariableListEntry::updateIsHovered()
{
    BaseMenuObject::updateIsHovered();

    if (label.containsMouse())
        label.updateIsHovered();
    if (control && control->containsMouse())
        control->updateIsHovered();
}

void zerokernel::VariableListEntry::recursiveSizeUpdate()
{
    BaseMenuObject::recursiveSizeUpdate();

    label.recursiveSizeUpdate();
    if (control)
        control->recursiveSizeUpdate();
}

bool zerokernel::VariableListEntry::handleSdlEvent(SDL_Event *event)
{
    if (label.handleSdlEvent(event))
        return true;
    if (control && control->handleSdlEvent(event))
        return true;
    return TreeListBaseEntry::handleSdlEvent(event);
}

void zerokernel::VariableListEntry::render()
{
    TreeListBaseEntry::render();

    label.render();
    if (control)
        control->render();
}

void zerokernel::VariableListEntry::setText(std::string text)
{
    label.setParent(this);
    label.set(std::move(text));
}

void zerokernel::VariableListEntry::setVariable(settings::IVariable *variable)
{
    switch (variable->getType())
    {
    case settings::VariableType::BOOL:
        control = std::move(createCheckbox(variable));
        break;
    case settings::VariableType::INT:
    case settings::VariableType::FLOAT:
        control = std::move(createSpinner(variable));
        break;
    case settings::VariableType::STRING:
        control = std::move(createTextInput(variable));
        break;
    case settings::VariableType::COLOR:
        control = std::move(createColorPicker(variable));
        break;
    case settings::VariableType::KEY:
        control = std::move(createKeyInput(variable));
        break;
    }
    if (control)
    {
        control->setParent(this);
        control->bb.setMargin(0, 0, 0, 4);
    }
}

std::unique_ptr<zerokernel::BaseMenuObject>
zerokernel::VariableListEntry::createCheckbox(settings::IVariable *variable)
{
    auto v = dynamic_cast<settings::Variable<bool> *>(variable);
    if (!v)
    {
        printf("WARNING: Could not cast to bool\n");
        return nullptr;
    }
    return std::make_unique<Checkbox>(*v);
}

std::unique_ptr<zerokernel::BaseMenuObject>
zerokernel::VariableListEntry::createSpinner(settings::IVariable *variable)
{
    if (variable->getType() == settings::VariableType::INT)
    {
        auto v = dynamic_cast<settings::Variable<int> *>(variable);
        if (!v)
        {
            printf("WARNING: Could not cast to int\n");
            return nullptr;
        }
        return std::make_unique<Spinner<int>>(*v);
    }
    else if (variable->getType() == settings::VariableType::FLOAT)
    {
        auto v = dynamic_cast<settings::Variable<float> *>(variable);
        if (!v)
        {
            printf("WARNING: Could not cast to float\n");
            return nullptr;
        }
        return std::make_unique<Spinner<float>>(*v);
    }
    return nullptr;
}

std::unique_ptr<zerokernel::BaseMenuObject>
zerokernel::VariableListEntry::createTextInput(settings::IVariable *variable)
{
    auto v = dynamic_cast<settings::Variable<std::string> *>(variable);
    if (!v)
    {
        printf("WARNING: Could not cast to string\n");
        return nullptr;
    }
    return std::make_unique<StringInput>(*v);
}

std::unique_ptr<zerokernel::BaseMenuObject>
zerokernel::VariableListEntry::createColorPicker(settings::IVariable *variable)
{
    auto v = dynamic_cast<settings::Variable<glez::rgba> *>(variable);
    if (!v)
    {
        printf("WARNING: Could not cast to color\n");
        return nullptr;
    }
    return std::make_unique<ColorSelector>(*v);
}

std::unique_ptr<zerokernel::BaseMenuObject>
zerokernel::VariableListEntry::createKeyInput(settings::IVariable *variable)
{
    auto v = dynamic_cast<settings::Variable<settings::Key> *>(variable);
    if (!v)
    {
        printf("WARNING: Could not cast to key\n");
        return nullptr;
    }
    return std::make_unique<InputKey>(*v);
}

zerokernel::VariableListEntry::VariableListEntry()
{
    bb.width.setFill();
    bb.height.setContent();
    bb.height.min = 12;
    bb.normalizeSize();
    label.bb.height.setFill();
    label.setParent(this);
}

void zerokernel::VariableListEntry::onMove()
{
    BaseMenuObject::onMove();

    label.onParentMove();
    if (control)
        control->onParentMove();
}

void zerokernel::VariableListEntry::recalculateSize()
{
    printf("VLE::recalculateSize()\n");
    bb.updateFillSize();
    bb.shrinkContent();
    bb.extend(label.getBoundingBox());
    if (control)
        bb.extend(control->getBoundingBox());
}

void zerokernel::VariableListEntry::emitSizeUpdate()
{
    if (control)
        control->move(bb.getContentBox().width -
                          control->getBoundingBox().getFullBox().width,
                      0);

    BaseMenuObject::emitSizeUpdate();
}

void zerokernel::VariableListEntry::setDepth(int depth)
{
    TreeListBaseEntry::setDepth(depth);

    label.move(depth * 5 + 4, 0);
}

void zerokernel::VariableListEntry::markPresentInUi()
{
    label.setColorText(&*marked_color);
}
