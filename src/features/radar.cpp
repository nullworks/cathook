
/*
 *
 * Nekohooks radar
 *
 */

#include "../util/catvars.hpp"
#include "../gui/base/CBaseWindow.hpp"

#include "radar.hpp"

namespace features::radar {

static CatEnum radar_menu({"Visuals", "Radar"});
static CatVarBool enabled(radar_menu, "radar", false, "Enable Radar");
static CatVarInt radar_size(radar_menu, "radar_size", 200, "Radar Size");
static CatEnum radar_type_enum({"Square", "Circle"});
static CatVarEnum radar_type(radar_menu, radar_type_enum, "radar_type", 0, "Radar Type");
/*
class CRadar : public CBaseWindow {
public:
virtual void Update();
virtual void Draw();
};

void CRadar::Update() {
  always_visible = enabled;
  size = std::make_pair(radar_size, radar_size);
}

void CRadar::Draw() {
  auto abs = AbsolutePosition();
  draw::RectFilled(abs.first, abs.second, size.first, size.second, colors::Transparent(colors::black, 0.9));
  draw::Rect(abs.first, abs.second, size.first, size.second, colors::pink);
}*/

void Init() {}

}
