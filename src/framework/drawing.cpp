
/*
 *
 *	Welcome to the draw module manager!
 *	Here is the modular drawing system you can tap into if needed!
 *	It is designed to only require draw line but you may add other specialized functions if needed.
 *		-Onee
 *
 */

// TODO, add texture func with texture load with drawline

#include <math.h>

#include "../util/mathlib.hpp" // Contains pi defines

#include "drawing.hpp"

namespace draw {

// Line
CMFunction<void(int, int, int, int, CatVector4)> Line {
	[](int x, int y, int w, int h, CatVector4 color) {}
};

// Outline rect
CMFunction<void(int, int, int, int, CatVector4)> Rect {
	[] (int x, int y, int w, int h, CatVector4 color) {
	// Make outline rect with draw line
	Line(x, y, w, 0, color); // Top
	Line(x, y + 1, 0, h, color); // Left
	Line(x + 1, y + h, w - 1, 0, color); // Botton
	Line(x + w, y + 1, 0, h - 1, color); // Right
}};

// Filled Rect
CMFunction<void(int, int, int, int, CatVector4)> RectFilled {
	[] (int x, int y, int w, int h, CatVector4 color) {
	// Make filled rect with lines
	for (int i = 0; i < w; i++) {
		Line(x + i, y, 0, h, color);
	}
}};

// Outline circle
CMFunction<void(int, int, float, int, CatVector4)> Circle {
	[](int x, int y, float radius, int steps, CatVector4 color) {
	if (radius < 0 || steps <= 3) return; // cant draw a cirle without specific parameters
	float px = 0;
	float py = 0;
	for (int i = 0; i < steps; i++) {
		float ang = 2 * PI * (float(i) / steps);
		float dx = x + radius * cos(ang);
		float dy = y + radius * sin(ang);
		if (!i) {
			ang = 2 * PI * (float(steps - 1) / steps);
			px = x + radius * cos(ang);
			py = y + radius * sin(ang);
		}
		Line(px, py, dx - px, dy - py, color);
		px = dx;
		py = dy;
	}
}};

// Filled circle
CMFunction<void(int, int, float, int, CatVector4)> CircleFilled {
	[](int x, int y, float radius, int steps, CatVector4 color) {
	if (radius < 0) return; // cant draw a cirle without specific parameters
	// Interate Left and right
	for (int i = 0; i < radius; i++) {
		// Up and down
		for (int ii = radius; ii >= 0; ii--) {
			// Check if our distance is within the radius
			if (sqrt(pow(i, 2) + pow(ii, 2)) > radius) continue;
			Line(x - i, y - ii, 0, ii * 2, color); // Left line
			Line(x + i, y - ii, 0, ii * 2, color); // Right line
			break;
		}
	}
}};

// String
CMFunction<void(const char*, int, int, int, int, CatVector4)> String {
	[](const char* text, int x, int y, int font, int size, CatVector4 color) {}};

// String length in pixels
CMFunction<std::pair<int, int>(const char*, int, int)> GetStringLength {
	[](const char* string, int font, int size) { return std::make_pair(0, 0); }};

// The main world to screen function used by most of the cheats esp features, sadly, this isnt going to be included into the draw object due to it being seperate
CMFunction<bool(CatVector, CatVector&)> WorldToScreen {
	[](CatVector, CatVector&) { return false; }};

const char* Fonts[] = {
	"Open Sans",
	"Bitstream Vera Sans Mono",
	"Unispace"
};
static CatEnum gui_menu({"GUI"});
CatVarEnum default_font(gui_menu, {"Open Sans", "Bitstream Vera Sans Mono", "Unispace"}, "gui_font", 0, "Gui font", "The main font setting");
CatVarInt default_font_size(gui_menu, "gui_font_size", 13, "Gui font size", "The main font size");

}






// Below here is ripped from the source sdk

// The goal is to get a world to screen matrix, to do that we need the following



// need pViewToProjection and pWorldToView

// pWorldToView
//



// BIG MATH LIB!!!

// Important for everythign
/*typedef float VMatrixRaw_t[4];
void MatrixMultiply( const VMatrix& src1, const VMatrix& src2, VMatrix& dst )
{
	// Make sure it works if src1 == dst or src2 == dst
	VMatrix tmp1, tmp2;
	const VMatrixRaw_t* s1 = (&src1 == &dst) ? tmp1.m : src1.m;
	const VMatrixRaw_t* s2 = (&src2 == &dst) ? tmp2.m : src2.m;

	if (&src1 == &dst)
	{
		MatrixCopy( src1, tmp1 );
	}
	if (&src2 == &dst)
	{
		MatrixCopy( src2, tmp2 );
	}

	dst[0][0] = s1[0][0] * s2[0][0] + s1[0][1] * s2[1][0] + s1[0][2] * s2[2][0] + s1[0][3] * s2[3][0];
	dst[0][1] = s1[0][0] * s2[0][1] + s1[0][1] * s2[1][1] + s1[0][2] * s2[2][1] + s1[0][3] * s2[3][1];
	dst[0][2] = s1[0][0] * s2[0][2] + s1[0][1] * s2[1][2] + s1[0][2] * s2[2][2] + s1[0][3] * s2[3][2];
	dst[0][3] = s1[0][0] * s2[0][3] + s1[0][1] * s2[1][3] + s1[0][2] * s2[2][3] + s1[0][3] * s2[3][3];

	dst[1][0] = s1[1][0] * s2[0][0] + s1[1][1] * s2[1][0] + s1[1][2] * s2[2][0] + s1[1][3] * s2[3][0];
	dst[1][1] = s1[1][0] * s2[0][1] + s1[1][1] * s2[1][1] + s1[1][2] * s2[2][1] + s1[1][3] * s2[3][1];
	dst[1][2] = s1[1][0] * s2[0][2] + s1[1][1] * s2[1][2] + s1[1][2] * s2[2][2] + s1[1][3] * s2[3][2];
	dst[1][3] = s1[1][0] * s2[0][3] + s1[1][1] * s2[1][3] + s1[1][2] * s2[2][3] + s1[1][3] * s2[3][3];

	dst[2][0] = s1[2][0] * s2[0][0] + s1[2][1] * s2[1][0] + s1[2][2] * s2[2][0] + s1[2][3] * s2[3][0];
	dst[2][1] = s1[2][0] * s2[0][1] + s1[2][1] * s2[1][1] + s1[2][2] * s2[2][1] + s1[2][3] * s2[3][1];
	dst[2][2] = s1[2][0] * s2[0][2] + s1[2][1] * s2[1][2] + s1[2][2] * s2[2][2] + s1[2][3] * s2[3][2];
	dst[2][3] = s1[2][0] * s2[0][3] + s1[2][1] * s2[1][3] + s1[2][2] * s2[2][3] + s1[2][3] * s2[3][3];

	dst[3][0] = s1[3][0] * s2[0][0] + s1[3][1] * s2[1][0] + s1[3][2] * s2[2][0] + s1[3][3] * s2[3][0];
	dst[3][1] = s1[3][0] * s2[0][1] + s1[3][1] * s2[1][1] + s1[3][2] * s2[2][1] + s1[3][3] * s2[3][1];
	dst[3][2] = s1[3][0] * s2[0][2] + s1[3][1] * s2[1][2] + s1[3][2] * s2[2][2] + s1[3][3] * s2[3][2];
	dst[3][3] = s1[3][0] * s2[0][3] + s1[3][1] * s2[1][3] + s1[3][2] * s2[2][3] + s1[3][3] * s2[3][3];
}


void MatrixBuildRotationAboutAxis( const Vector &vAxisOfRot, float angleDegrees, matrix3x4_t &dst )
{
	float radians;
	float axisXSquared;
	float axisYSquared;
	float axisZSquared;
	float fSin;
	float fCos;

	radians = angleDegrees * ( M_PI / 180.0 );
	fSin = sin( radians );
	fCos = cos( radians );

	axisXSquared = vAxisOfRot[0] * vAxisOfRot[0];
	axisYSquared = vAxisOfRot[1] * vAxisOfRot[1];
	axisZSquared = vAxisOfRot[2] * vAxisOfRot[2];

	// Column 0:
	dst[0][0] = axisXSquared + (1 - axisXSquared) * fCos;
	dst[1][0] = vAxisOfRot[0] * vAxisOfRot[1] * (1 - fCos) + vAxisOfRot[2] * fSin;
	dst[2][0] = vAxisOfRot[2] * vAxisOfRot[0] * (1 - fCos) - vAxisOfRot[1] * fSin;

	// Column 1:
	dst[0][1] = vAxisOfRot[0] * vAxisOfRot[1] * (1 - fCos) - vAxisOfRot[2] * fSin;
	dst[1][1] = axisYSquared + (1 - axisYSquared) * fCos;
	dst[2][1] = vAxisOfRot[1] * vAxisOfRot[2] * (1 - fCos) + vAxisOfRot[0] * fSin;

	// Column 2:
	dst[0][2] = vAxisOfRot[2] * vAxisOfRot[0] * (1 - fCos) + vAxisOfRot[1] * fSin;
	dst[1][2] = vAxisOfRot[1] * vAxisOfRot[2] * (1 - fCos) - vAxisOfRot[0] * fSin;
	dst[2][2] = axisZSquared + (1 - axisZSquared) * fCos;

	// Column 3:
	dst[0][3] = 0;
	dst[1][3] = 0;
	dst[2][3] = 0;
}


// Build for compute view matrices in 2nd step
void MatrixBuildPerspectiveX( VMatrix& dst, double flFovX, double flAspect, double flZNear, double flZFar )
{
	float flWidth = 2.0f * flZNear * tanf( flFovX * M_PI / 360.0f );
	float flHeight = flWidth / flAspect;
	dst.Init(   2.0f * flZNear / flWidth, 0.0f,	0.0f, 0.0f,	0.0f, 2.0f * flZNear/ flHeight,	0.0f, 0.0f, 0.0f, 0.0f, flZFar / ( flZNear - flZFar ), flZNear * flZFar / ( flZNear - flZFar ), 0.0f, 0.0f,	-1.0f, 0.0f );
}

void ComputeViewMatrix( VMatrix *pViewMatrix, const Vector &origin, const QAngle &angles )
{
	static VMatrix baseRotation;
	static bool bDidInit;
	if ( !bDidInit )
	{
		MatrixBuildRotationAboutAxis( baseRotation, Vector( 1, 0, 0 ), -90 );
		MatrixRotate( baseRotation, Vector( 0, 0, 1 ), 90 );
		bDidInit = true;
	}

	*pViewMatrix = baseRotation;
	MatrixRotate( *pViewMatrix, Vector( 1, 0, 0 ), -angles[2] );
	MatrixRotate( *pViewMatrix, Vector( 0, 1, 0 ), -angles[0] );
	MatrixRotate( *pViewMatrix, Vector( 0, 0, 1 ), -angles[1] );

	MatrixTranslate( *pViewMatrix, -origin );
}

float ComputeViewMatrices( VMatrix *pWorldToView, VMatrix *pViewToProjection, VMatrix *pWorldToProjection, const CViewSetup &viewSetup )
{
	ComputeViewMatrix( pWorldToView, viewSetup.origin, viewSetup.angles );

	// Get aspect ratio here from height and width
	flAspectRatio = (viewSetup.height != 0) ? ( (float)viewSetup.width / (float)viewSetup.height ) : 1.0f;

	MatrixBuildPerspectiveX( *pViewToProjection, viewSetup.fov, flAspectRatio, viewSetup.zNear, viewSetup.zFar );

	//			    have from build per								Need
	//			  Still need aspect ratio
	MatrixMultiply( *pViewToProjection, *pWorldToView, *pWorldToProjection );

	return flAspectRatio;
}



void ComputeWorldToScreenMatrix( VMatrix *pWorldToScreen, const VMatrix &worldToProjection, const CViewSetup &viewSetup )
{
	VMatrix offsetToPixels( viewSetup.width, 0.0f, 0.0f, viewSetup.x,
							0.0f, viewSetup.height, 0.0f, viewSetup.y,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f );

	VMatrix projectionToPixels;
	MatrixMultiply( offsetToPixels, g_ProjectionToOffset, projectionToPixels );
	MatrixMultiply( projectionToPixels, worldToProjection, *pWorldToScreen );
}
*/
