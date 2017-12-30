
/*
 *
 *  An easy way to display information to the user.
 *  Make sure to set max_size to have clamping
 *
 */

#include "../../util/stringhelpers.hpp"
#include "../../util/colors.hpp"
#include "../../framework/input.hpp" // For mouse location
#include "../../framework/drawing.hpp"

#include "CBaseInfoBox.hpp"

namespace gui { namespace base {

CBaseInfoBox::CBaseInfoBox(const char* _name, const char* info) : CBaseWidget(_name), infostring(info) {
  position_mode = ABSOLUTE;
}

void CBaseInfoBox::Draw() {

  // Get tooltip size
  auto infostring_size = draw::GetStringLength(infostring.c_str(), 1, 20);

  // Check if we need to wrap
  if (infostring_size.first > max_size.first) {

    std::string wrapped_str; // A place to store the wrapped string, with the size ;)
    std::string cur_line; // Our current line

    // Seperate our strings to make things easy
    auto sep_strs = sepstr(infostring);

    // Here we take each word and make it fit in our box.
    for (const auto& cur_str : sep_strs) {

      // If it goes over max width, we add the current line to the ourput, then return
      if (draw::GetStringLength((cur_line + ' ' + cur_str).c_str(), 1, 20).first > max_size.first) {
        // Add the current line
        wrapped_str += cur_line + '\n';

        // Set the new line to our current string
        cur_line = cur_str;

        // continue to the next string
        continue;
      }

      // Add the current string to the line
      cur_line += ' ' + cur_str;
    }
    // Here we can set the new size and stuff.
    infostring_size = draw::GetStringLength(wrapped_str.c_str(), 1, 20);
    infostring = wrapped_str;
  }
  // Draw string
  draw::RectFilled(input::mouse.first, input::mouse.second, infostring_size.first + 4, infostring_size.second + 4, colors::Transparent(colors::black));
  draw::Rect(input::mouse.first, input::mouse.second, infostring_size.first + 4, infostring_size.second + 4, colors::pink);
  draw::String(infostring.c_str(), input::mouse.first + 2, input::mouse.second + 2, 1, 20, colors::white);
}

}}
