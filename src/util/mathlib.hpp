 
/*
 *
 *	An area to put all that good delicious math that can work crossgame, or in some of the framework!
 *		Dont put anything game specific here please!
 *			-Onee
 *
 */


// Struct for point in 3d space
// I stole it from imgui, but it works too good
struct CatVector { 
	float x, y, z; 
	CatVector(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { 
		x = _x; y = _y; z = _z; 
	} 
};