
/*
 *
 * Nekohooks radar
 *
 */

#include "../util/catvars.hpp"
#include "../gui/base/CBaseWidget.hpp"
#include "radar.hpp"

static CatEnum radar_menu({"Visuals", "Radar"});
CatVarBool enabled(radar_menu, "radar", false, "Enable Radar");
CatVarInt radar_size(radar_menu, "radar_size", 200, "Radar Size");
CatVarEnum radar_type(radar_menu, {"Square", "Circle"}, "radar_type", 0, "Radar Type");

class CRadar {
public:
virtual void Update();
virtual void Draw();
};

void CRadar::Update() {

}

void CRadar::Draw() {

}
