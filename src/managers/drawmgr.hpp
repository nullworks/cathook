

namespace drawmgr {
	
void DrawTick(); // So that a hooked function from a module can call this
	
// Used by various parts of code to request to have its function run by the draw manager
void RequestDrawOnBefore(void(*func)(void));
void RequestDrawOnDraw(void(*func)(void));
void RequestDrawOnAfter(void(*func)(void));
}

