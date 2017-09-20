 

#include "../Source_2013_SDK/sdk.h"

// Contains the original func
typedef bool(*CreateMove_t)(void*, float, CUserCmd*);
typedef void(*PaintTraverse_t)(void*, unsigned int, bool, bool);

bool CreateMove_hook(void*, float, CUserCmd*);
void PaintTraverse_hook(void*, unsigned int, bool, bool);