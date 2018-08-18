/*
  Created by Jenny White on 13.05.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

namespace zerokernel
{

class Message;

class IMessageHandler
{
public:
    virtual void handleMessage(Message &msg, bool is_relayed) = 0;
};
} // namespace zerokernel