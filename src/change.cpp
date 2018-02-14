/*
 * Created on: Fed 14. 2018
 * Author: LIghtyPonce
 */

#include "common.hpp"

#include "sdk.hpp"

void enableGUI() // needed, otherwise tf2 crashes upon attaching
{   
    if (GUI)
    {
        gui_visible = true;
    }
    else
    {
        gui_visible = false;
    }
}