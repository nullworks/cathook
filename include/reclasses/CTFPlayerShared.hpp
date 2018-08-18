#include "reclasses.hpp"

namespace re
{

class CTFPlayerShared
{
public:
    inline static float GetCritMult(CTFPlayerShared *self)
    {
        return ((fminf(fmaxf(*(float *) (unsigned(self) + 672) * 0.0039215689f,
                             0.0f),
                       1.0f) *
                 3.0f) +
                1.0f);
    }
    inline static bool IsCritBoosted(CTFPlayerShared *self)
    {
        // TODO signature
        return false;
    }
};
} // namespace re
