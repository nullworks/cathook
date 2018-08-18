/*
  Created on 26.07.18.
*/

#pragma once

#include <settings/Settings.hpp>
#include <menu/object/Text.hpp>
#include <menu/special/TreeListBaseEntry.hpp>

namespace zerokernel
{

class VariableListEntry : public TreeListBaseEntry
{
public:
    VariableListEntry();

    ~VariableListEntry() override = default;

    void updateIsHovered() override;

    void recursiveSizeUpdate() override;

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    void onMove() override;

    void recalculateSize() override;

    void emitSizeUpdate() override;

    //

    void setText(std::string text);

    void setVariable(settings::IVariable *variable);

    void setDepth(int depth) override;

    void markPresentInUi();

    std::unique_ptr<BaseMenuObject>
    createCheckbox(settings::IVariable *variable);

    std::unique_ptr<BaseMenuObject>
    createSpinner(settings::IVariable *variable);

    std::unique_ptr<BaseMenuObject>
    createTextInput(settings::IVariable *variable);

    std::unique_ptr<BaseMenuObject>
    createColorPicker(settings::IVariable *variable);

    std::unique_ptr<BaseMenuObject>
    createKeyInput(settings::IVariable *variable);

    //

    std::unique_ptr<BaseMenuObject> control{ nullptr };
    Text label{};
};
} // namespace zerokernel