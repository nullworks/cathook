


namespace worldtick {

void WorldTickRun();

// Init these to have your function be called on worldtick
void RequestBeforeWTick(void(*func)(void));
void RequestWTick(void(*func)(void));
void RequestAfterWTick(void(*func)(void));
}
