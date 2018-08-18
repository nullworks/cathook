/*
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <menu/Message.hpp>

namespace zerokernel
{

Message::Message(std::string name) : name(name), kv(std::move(name))
{
}
} // namespace zerokernel