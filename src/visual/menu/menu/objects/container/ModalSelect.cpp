/*
  Created on 27.07.18.
*/

#include <menu/object/container/ModalSelect.hpp>
#include <menu/Message.hpp>

zerokernel::ModalSelect::ModalSelect() : ModalContainer{}
{
    bb.height.setContent();
}

void zerokernel::ModalSelect::handleMessage(zerokernel::Message &msg,
                                            bool is_relayed)
{
    BaseMenuObject::handleMessage(msg, is_relayed);

    if (!is_relayed && msg.name == "LeftClick")
    {
        auto option = dynamic_cast<Option *>(msg.sender);
        if (option == nullptr)
        {
            printf("WARNING: ButtonClicked in ModalSelect, but sender is not "
                   "Option\n");
            return;
        }
        Message os{ "OptionSelected" };
        os.kv["value"] = option->value;
        emit(os, false);
        modal.close();
    }
}

void zerokernel::ModalSelect::addOption(std::string name, std::string value,
                                        std::optional<std::string> tooltip)
{
    auto option     = std::make_unique<Option>(name, value);
    option->tooltip = std::move(tooltip);
    option->addMessageHandler(*this);
    addObject(std::move(option));
}

void zerokernel::ModalSelect::reorderElements()
{
    int acc{ 0 };
    for (auto &o : objects)
    {
        o->move(0, o->getBoundingBox().margin.top + acc);
        acc += o->getBoundingBox().getFullBox().height;
    }
}
