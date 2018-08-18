/*
  Created on 29.07.18.
*/

#pragma once

union SDL_Event;

namespace gui
{

void init();
void draw();
bool handleSdlEvent(SDL_Event *event);

void onLevelLoad();
} // namespace gui