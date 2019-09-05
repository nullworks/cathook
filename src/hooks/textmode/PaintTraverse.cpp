#include "common.hpp"
#include "HookedMethods.hpp"
namespace hooked_methods
{
DEFINE_HOOKED_METHOD(PaintTraverse, void, vgui::IPanel *, unsigned int, bool, bool)
{
    return;
}
} // namespace hooked_methods
