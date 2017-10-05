 
/*
 *
 *	This is the world to screen framework.
 *	!!!!TODO!!!!, Make a world to screen conversion utility that uses a small amount of info to get wts
 *
 *
 */

#include "../logging.h"
#include "worldtoscreen.hpp"

namespace wts {
	
// The main world to screen function used by most of the cheats esp features
bool(*StoredWorldToScreen)(CatVector&, CatVector&) = nullptr;
bool WorldToScreen(CatVector& world, CatVector& screen) {
	if (StoredWorldToScreen == nullptr) return false;
	
	return StoredWorldToScreen(world, screen);
	//return false; // We cant do this ourself quite yet sadly...
}
void InitWorldToScreen(bool *funke(CatVector&, CatVector&)) {
	StoredWorldToScreen = funke;
}

// Refreshes world to screen if avaible
//void(*StoredRefresh)() = nullptr;
void Refresh() {
	//if (StoredRefresh != nullptr) StoredRefresh();
}
/*void InitRefresh(void *func()) {
	StoredRefresh = func;
}*/
	
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