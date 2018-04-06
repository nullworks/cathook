
/*
 *
 *  An easy way to display information to the user.
 *  Make sure to set max_size to have clamping
 *
 */

#include "../../util/strings.hpp"
#include "../../util/colors.hpp"
#include "../../framework/input.hpp" // For mouse location
#include "../../framework/drawing.hpp"

#include "CBaseInfoBox.hpp"

namespace gui { namespace base {

CBaseInfoBox::CBaseInfoBox(std::string name, std::string info, std::pair<int, int> max_size) : CBaseWidget(name), infostring(info), max_size(max_size) {
}

void CBaseInfoBox::Draw() {

  /*// Get tooltip size
  auto infostring_size = draw::GetStringLength(infostring.c_str(), 1, 20);

  // Check if we need to wrap
  if (max_size.first!=-1&&infostring_size.first > max_size.first) {

    std::string wrapped_str; // A place to store the wrapped string, with the size ;)
    std::string cur_line; // Our current line

    // Seperate our strings to make things easy
    auto sep_strs = sepstr(infostring);

    // Here we take each word and make it fit in our box.
    for (const auto& cur_str : sep_strs) {

      // If it goes over max width, we add the current line to the ourput, then return
      if (draw::GetStringLength((cur_line + ' ' + cur_str).c_str(), draw::default_font, draw::default_font_size).first > max_size.first) {
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
    infostring_size = draw::GetStringLength(wrapped_str.c_str(), draw::default_font, draw::default_font_size);
    infostring = wrapped_str;
  }
  // Draw string
  auto mouse = input::GetMouse();
  draw::RectFilled(mouse.first, mouse.second, infostring_size.first + 4, infostring_size.second + 4, colors::Transparent(colors::black));
  draw::Rect(mouse.first, mouse.second, infostring_size.first + 4, infostring_size.second + 4, colors::pink);
  draw::String(infostring.c_str(), mouse.first + 2, mouse.second + 2, draw::default_font_size, draw::default_font_size, colors::white);*/
}

}}
