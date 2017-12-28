
/*
 *
 *  An easy way to display information to the user.
 *  Make sure to set max_size to have clamping
 *
 */

#include "../../framework/input.hpp" // For mouse location

#include "CBaseTooltip.hpp"

// Draw tooltip on mouse, and put size and offset somewhere else to prevent child by point from going off
// Make sure the root hover on works, and that it will recurse
namespace gui { namespace base {

CBaseTooltip::CBaseTooltip(const char* _name) : CBaseWidget(_name) {
  position_mode = FLOATING; // We dont want anything to move us
  // Here we want to set the size and offset to something that ChildByPoint wont find
  offset = std::make_pair(-128, -128);
  size = std::make_pair(0, 0);
}

// as well as set our zindex to always be on top
void CBaseTooltip::Update() {
  zindex = -100;
}

void CBaseTooltip::Draw() {
  /*//input::mouse;

  // Get tooltip size
  auto tooltip_size = draw::GetStringLength(tooltip.c_str(), 1, 20);

  // Check if we need to wrap
  if (tooltip_size.first > max_size.first) {

    // A place to store the wrapped string, with the size ;)
    std::pair<std::pair<int, int>, std::string> wrapped_str;

    // Our current line
    std::pair<std::pair<int, int>, std::string> cur_line;

    // Seperate our strings to make things easy
    auto tmp = sepstr(tooltip);

    // Iterate and wrap text
    for (const auto& cur_str : tmp) {
      // Get size
      auto cur_line_size = draw::GetStringLength((cur_line + ' ' + cur_str).c_str(), 1, 20);
      // Check if it goes over our width
      if (cur_line_size.first > max_size.first) {
        // Add the current line with newline
        wrapped_str.second += cur_str + '\n';
        wrapped_str.first = std::max
        // Set the new line to our current string
        cur_line = std::make_pair(cur_line_size, cur_str);
      }

      // Add the current string to the line
      cur_line += ' ' + cur_str;
    }


    return;
  }
  // Draw unwrapped string
  draw::RectFilled(input::mouse.first, input::mouse.second, tooltip_size.first + 4, tooltip_size.second + 4, Transparent(colors::black));
  draw::Rect(input::mouse.first, input::mouse.second, tooltip_size.first + 4, tooltip_size.second + 4, colors::pink);
  draw::String(tooltip.c_str(), input::mouse.first + 2, input::mouse.second + 2, 1, 20, colors::white);*/
}

}}
