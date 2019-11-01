#include "common.hpp"
#include "sdk.hpp"

// Taken from source SDK
void AngleQuaternion(const RadianEuler &angles, Quaternion &outQuat)
{
    float sr, sp, sy, cr, cp, cy;

    SinCos(angles.z * 0.5f, &sy, &cy);
    SinCos(angles.y * 0.5f, &sp, &cp);
    SinCos(angles.x * 0.5f, &sr, &cr);

    // NJS: for some reason VC6 wasn't recognizing the common subexpressions:
    float srXcp = sr * cp, crXsp = cr * sp;
    outQuat.x = srXcp * cy - crXsp * sy; // X
    outQuat.y = crXsp * cy + srXcp * sy; // Y

    float crXcp = cr * cp, srXsp = sr * sp;
    outQuat.z = crXcp * sy - srXsp * cy; // Z
    outQuat.w = crXcp * cy + srXsp * sy; // W (real component)
}
void AngleQuaternion(const QAngle &angles, Quaternion &outQuat)
{
    float sr, sp, sy, cr, cp, cy;
    SinCos(DEG2RAD(angles.y) * 0.5f, &sy, &cy);
    SinCos(DEG2RAD(angles.x) * 0.5f, &sp, &cp);
    SinCos(DEG2RAD(angles.z) * 0.5f, &sr, &cr);

    // NJS: for some reason VC6 wasn't recognizing the common subexpressions:
    float srXcp = sr * cp, crXsp = cr * sp;
    outQuat.x = srXcp * cy - crXsp * sy; // X
    outQuat.y = crXsp * cy + srXcp * sy; // Y

    float crXcp = cr * cp, srXsp = sr * sp;
    outQuat.z = crXcp * sy - srXsp * cy; // Z
    outQuat.w = crXcp * cy + srXsp * sy; // W (real component)
}
void QuaternionSlerp(const Quaternion &p, const Quaternion &q, float t, Quaternion &qt)
{
    Quaternion q2;
    // 0.0 returns p, 1.0 return q.

    // decide if one of the quaternions is backwards
    QuaternionAlign(p, q, q2);

    QuaternionSlerpNoAlign(p, q2, t, qt);
}
void QuaternionAlign(const Quaternion &p, const Quaternion &q, Quaternion &qt)
{
    // FIXME: can this be done with a quat dot product?

    int i;
    // decide if one of the quaternions is backwards
    float a = 0;
    float b = 0;
    for (i = 0; i < 4; i++)
    {
        a += (p[i] - q[i]) * (p[i] - q[i]);
        b += (p[i] + q[i]) * (p[i] + q[i]);
    }
    if (a > b)
    {
        for (i = 0; i < 4; i++)
        {
            qt[i] = -q[i];
        }
    }
    else if (&qt != &q)
    {
        for (i = 0; i < 4; i++)
        {
            qt[i] = q[i];
        }
    }
}
void QuaternionSlerpNoAlign(const Quaternion &p, const Quaternion &q, float t, Quaternion &qt)
{
    float omega, cosom, sinom, sclp, sclq;
    int i;

    // 0.0 returns p, 1.0 return q.

    cosom = p[0] * q[0] + p[1] * q[1] + p[2] * q[2] + p[3] * q[3];

    if ((1.0f + cosom) > 0.000001f)
    {
        if ((1.0f - cosom) > 0.000001f)
        {
            omega = acos(cosom);
            sinom = sin(omega);
            sclp  = sin((1.0f - t) * omega) / sinom;
            sclq  = sin(t * omega) / sinom;
        }
        else
        {
            // TODO: add short circuit for cosom == 1.0f?
            sclp = 1.0f - t;
            sclq = t;
        }
        for (i = 0; i < 4; i++)
        {
            qt[i] = sclp * p[i] + sclq * q[i];
        }
    }
    else
    {
        qt[0] = -q[1];
        qt[1] = q[0];
        qt[2] = -q[3];
        qt[3] = q[2];
        sclp  = sin((1.0f - t) * (0.5f * M_PI));
        sclq  = sin(t * (0.5f * M_PI));
        for (i = 0; i < 3; i++)
        {
            qt[i] = sclp * p[i] + sclq * qt[i];
        }
    }
}
void QuaternionAngles(const Quaternion &q, QAngle &angles)
{
    // FIXME: doing it this way calculates too much data, needs to do an optimized version...
    matrix3x4_t matrix;
    QuaternionMatrix(q, matrix);
    MatrixAngles(matrix, angles);

    Assert(angles.IsValid());
}
void QuaternionMatrix(const Quaternion &q, matrix3x4_t &matrix)
{

    // Original code
    // This should produce the same code as below with optimization, but looking at the assmebly,
    // it doesn't.  There are 7 extra multiplies in the release build of this, go figure.
    matrix[0][0] = 1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z;
    matrix[1][0] = 2.0 * q.x * q.y + 2.0 * q.w * q.z;
    matrix[2][0] = 2.0 * q.x * q.z - 2.0 * q.w * q.y;

    matrix[0][1] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
    matrix[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
    matrix[2][1] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;

    matrix[0][2] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
    matrix[1][2] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
    matrix[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;

    matrix[0][3] = 0.0f;
    matrix[1][3] = 0.0f;
    matrix[2][3] = 0.0f;
}
void MatrixAngles(const matrix3x4_t &matrix, float *angles)
{
    float forward[3];
    float left[3];
    float up[3];

    //
    // Extract the basis vectors from the matrix. Since we only need the Z
    // component of the up vector, we don't get X and Y.
    //
    forward[0] = matrix[0][0];
    forward[1] = matrix[1][0];
    forward[2] = matrix[2][0];
    left[0]    = matrix[0][1];
    left[1]    = matrix[1][1];
    left[2]    = matrix[2][1];
    up[2]      = matrix[2][2];

    float xyDist = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);

    // enough here to get angles?
    if (xyDist > 0.001f)
    {
        // (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
        angles[1] = RAD2DEG(atan2f(forward[1], forward[0]));

        // (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
        angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));

        // (roll)	z = ATAN( left.z, up.z );
        angles[2] = RAD2DEG(atan2f(left[2], up[2]));
    }
    else // forward is mostly Z, gimbal lock-
    {
        // (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
        angles[1] = RAD2DEG(atan2f(-left[0], left[1]));

        // (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
        angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));

        // Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
        angles[2] = 0;
    }
}
