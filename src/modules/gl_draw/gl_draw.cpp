
/*
 *
 *
 * Gl Draw using libglez
 *
 *
 */

#include "../../frameworks/gameticks.hpp" // Run on draw
#include "../../f"
#include "../../frameworks/input.hpp" // For bounds

namespace modules { namespace gl_draw {

// Convert frameworks colors into glez colors
static inline glez_rgba_t convert(const CatVector4& in) {
  return glez_rgba_t(in.x, in.y, in.z, in.a);
}

// a fast way to convert float to int... is le paste but im not letting slow hurt me >_>
static inline int float2int(double d) {
   union Cast {
      double d;
      long l;
    };
   volatile Cast c;
   c.d = d + 6755399441055744.0;
   return c.l;
}

// Font system, We only use 12 fonts to save on memory
//static glez_rgba_t glez_fonts[FONT_COUNT][12];

// Retrieves one of the fonts generated and saved into
// TODO fix
/*static const glez_rgba_t& GetGlezFont(const int& font, const int& size) {
	// Init fonts if we havent
	static bool font_init = false;
	if (!font_init) {
		// Per font
		for (int i = 0; i < FONT_COUNT; i++) {
			// Per size
			for (int ii = 0; ii < 12; ii++) {
				surface_fonts[i][ii] = g_ISurface->CreateFont();
				g_ISurface->SetFontGlyphSet(surface_fonts[i][ii], draw::Fonts[i], ii + 20, 0, 0, 0, 0);
			}
		}
		font_init = true;
	}
	// Clamping as we dont have that many font sizes
	return glez_fonts[font][std::max(std::min(32, size), 20) - 1];
}*/

// Run to init glez
void Init() {

  // Setup glez
  glez_init(input::bounds.first, input::bounds.second);

  // Glez draw before
  auto last_bounds = input::bounds;
  drawmgr_before([](){

    // If our last bounds changed, we need to let glez know about the change
    if (last_bounds != input::bounds) {
      glez_resize(input::bounds.first, input::bounds.second);
      last_bounds = input::bounds;
    }

    // Let glez know we are starting draw
    glez_begin();
  });

  // Glez draw before, Let glez know we are ending draw
  drawmgr_after(glez_after);

  // Give the drawmgr our glez draw functions
  draw::Line = [](const int& x, const int& y, const int& w, const int& h, const CatVector4& color) { glez_line(x, y, w, h, convert(color), 1); };
  draw::Rect = [](const int& x, const int& y, const int& w, const int& h, const CatVector4& color) { glez_rect_outline(x, y, w, h, convert(color), 1); };
  draw::RectFilled = [](const int& x, const int& y, const int& w, const int& h, const CatVector4& color) { glez_rect(x, y, w, h, convert(color)); };

  draw::Circle = [](const int& x, const int& y, const float& radius, const int& steps, const CatVector4& color){ glez_circle(x, y, radius, convert(color), 1, steps); };

  /*draw::String = [](const char* text, const int& x, const int& y, const int& font, const int& size, const CatVector4& color) {
    float tmp1, tmp2;
    glez_string_with_outline(x, y, text, font, convert(color), convert(colors::black), 2, 100, tmp1, tmp2);
  };
  draw::GetStringLength = [](const char* string, const int& font, const int& size) {
    float tmp1, tmp2;
    glez_font_string_size(font, string, tmp1, tmp2);
    return std::make_pair(float2int(tmp1) + 2, float2int(tmp2) + 2);
  };*/
}

}}
