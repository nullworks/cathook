/*
 * classinfo.cpp
 *
 *  Created on: May 13, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

client_classes::dummy *client_class_list = nullptr;

#define INST_CLIENT_CLASS_LIST(x) client_classes::x client_classes::x##_list

INST_CLIENT_CLASS_LIST(tf2);

void InitClassTable()
{
    client_class_list = (client_classes::dummy *) &client_classes::tf2_list;
    if (!client_class_list)
    {
        logging::Info("FATAL: Cannot initialize class list! Game will crash if "
                      "cathook is enabled.");
        // cathook = false;
    }
}
