/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_Math.h
Content     :   Implementation of 3D primitives such as vectors, matrices.
Created     :   September 4, 2012
Authors     :   Andrew Reisse, Michael Antonov, Steve LaValle, Anna Yershova

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_Math_h
#define OVR_Math_h

#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "OVR_Types.h"
#include "OVR_RefCount.h"

namespace OVR {

//-------------------------------------------------------------------------------------
// Constants for 3D world/axis definitions.

// Definitions of axes for coordinate and rotation conversions.
enum Axis
{
    Axis_X = 0, Axis_Y = 1, Axis_Z = 2
};

// RotateDirection describes the rotation direction around an axis, interpreted as follows:
//  CW  - Clockwise while looking "down" from positive axis towards the origin.
//  CCW - Counter-clockwise while looking from the positive axis towards the origin,
//        which is in the negative axis direction.
//  CCW is the default for the RHS coordinate system. Oculus standard RHS coordinate
//  system defines Y up, X right, and Z back (pointing out from the screen). In this
//  system Rotate_CCW around Z will specifies counter-clockwise rotation in XY plane.
enum RotateDirection
{
    Rotate_CCW = 1,
    Rotate_CW  = -1 
};

enum HandedSystem
{
    Handed_R = 1, Handed_L = -1
};

// AxisDirection describes which way the axis points. Used by WorldAxes.
enum AxisDirection
{
    Axis_Up    =  2,
    Axis_Down  = -2,
    Axis_Right =  1,
    Axis_Left  = -1,
    Axis_In    =  3,
    Axis_Out   = -3
};

struct WorldAxes
{
    AxisDirection XAxis, YAxis, ZAxis;

    WorldAxes(AxisDirection x, AxisDirection y, AxisDirection z)
        : XAxis(x), YAxis(y), ZAxis(z) 
    { OVR_ASSERT(abs(x) != abs(y) && abs(y) != abs(z) && abs(z) != abs(x));}
};


//-------------------------------------------------------------------------------------
// ***** Math

// Math class contains constants and functions. This class is a template specialized
// per type, with Math<float> and Math<double> being distinct.
template<class Type>
class Math
{  
};

// Single-precision Math constants class.
template<>
class Math<float>
{
public:
    static const float Pi;
    static const float TwoPi;
    static const float PiOver2;
    static const float PiOver4;
    static const float E;

    static const float MaxValue;          // Largest positive float Value
    static const float MinPositiveValue;  // Smallest possible positive value

    static const float RadToDegreeFactor;
    static const float DegreeToRadFactor;

    static const float Tolerance; //  0.00001f;
    static const float SingularityRadius; //0.00000000001f for Gimbal lock numerical problems
};

// Double-precision Math constants class.
template<>
class Math<double>
{
public:
    static const double Pi;
    static const double TwoPi;
    static const double PiOver2;
    static const double PiOver4;
    static const double E;

    static const double MaxValue;          // Largest positive double Value
    static const double MinPositiveValue;  // Smallest possible positive value

    static const double RadToDegreeFactor;
    static const double DegreeToRadFactor;

    static const double Tolerance; //  0.00001f;
    static const double SingularityRadius; //0.00000000001 for Gimbal lock numerical problems
};

typedef Math<float>  Mathf;
typedef Math<double> Mathd;

// Conversion functions between degrees and radians
template<class FT>
FT RadToDegree(FT rads) { return rads * Math<FT>::RadToDegreeFactor; }
template<class FT>
FT DegreeToRad(FT rads) { return rads * Math<FT>::DegreeToRadFactor; }

template<class T>
class Quat;

//-------------------------------------------------------------------------------------
// ***** Vector2f - 2D Vector2f

// Vector2f represents a 2-dimensional vector or point in space,
// consisting of coordinates x and y,

template<class T>
class Vector2
{
public:
    T x, y;

    Vector2() : x(0), y(0) { }
    Vector2(T x_, T y_) : x(x_), y(y_) { }
    explicit Vector2(T s) : x(s), y(s) { }

    bool     operator== (const Vector2& b) const  { return x == b.x && y == b.y; }
    bool     operator!= (const Vector2& b) const  { return x != b.x || y != b.y; }
             
    Vector2  operator+  (const Vector2& b) const  { return Vector2(x + b.x, y + b.y); }
    Vector2& operator+= (const Vector2& b)        { x += b.x; y += b.y; return *this; }
    Vector2  operator-  (const Vector2& b) const  { return Vector2(x - b.x, y - b.y); }
    Vector2& operator-= (const Vector2& b)        { x -= b.x; y -= b.y; return *this; }
    Vector2  operator- () const                   { return Vector2(-x, -y); }

    // Scalar multiplication/division scales vector.
    Vector2  operator*  (T s) const               { return Vector2(x*s, y*s); }
    Vector2& operator*= (T s)                     { x *= s; y *= s; return *this; }

    Vector2  operator/  (T s) const               { T rcp = T(1)/s;
                                                    return Vector2(x*rcp, y*rcp); }
    Vector2& operator/= (T s)                     { T rcp = T(1)/s;
                                                    x *= rcp; y *= rcp;
                                                    return *this; }

    // Compare two vectors for equality with tolerance. Returns true if vectors match withing tolerance.
    bool      Compare(const Vector2&b, T tolerance = Mathf::Tolerance)
    {
        return (fabs(b.x-x) < tolerance) && (fabs(b.y-y) < tolerance);
    }
    
    // Dot product overload.
    // Used to calculate angle q between two vectors among other things,
    // as (A dot B) = |a||b|cos(q).
    T     operator*  (const Vector2& b) const    { return x*b.x + y*b.y; }

    // Returns the angle from this vector to b, in radians.
    T       Angle(const Vector2& b) const        { return acos((*this * b)/(Length()*b.Length())); }

    // Return Length of the vector squared.
    T       LengthSq() const                     { return (x * x + y * y); }
    // Return vector length.
    T       Length() const                       { return sqrt(LengthSq()); }

    // Returns distance between two points represented by vectors.
    T       Distance(Vector2& b) const           { return (*this - b).Length(); }
    
    // Determine if this a unit vector.
    bool    IsNormalized() const                 { return fabs(LengthSq() - T(1)) < Math<T>::Tolerance; }
    // Normalize, convention vector length to 1.    
    void    Normalize()                          { *this /= Length(); }
    // Returns normalized (unit) version of the vector without modifying itself.
    Vector2 Normalized() const                   { return *this / Length(); }

    // Linearly interpolates from this vector to another.
    // Factor should be between 0.0 and 1.0, with 0 giving full value to this.
    Vector2 Lerp(const Vector2& b, T f) const    { return *this*(T(1) - f) + b*f; }

    // Projects this vector onto the argument; in other words,
    // A.Project(B) returns projection of vector A onto B.
    Vector2 ProjectTo(const Vector2& b) const    { return b * ((*this * b) / b.LengthSq()); }
};


typedef Vector2<float>  Vector2f;
typedef Vector2<double> Vector2d;

//-------------------------------------------------------------------------------------
// ***** Vector3f - 3D Vector3f

// Vector3f represents a 3-dimensional vector or point in space,
// consisting of coordinates x, y and z.

template<class T>
class Vector3
{
public:
    T x, y, z;

    Vector3() : x(0), y(0), z(0) { }
    Vector3(T x_, T y_, T z_ = 0) : x(x_), y(y_), z(z_) { }
    explicit Vector3(T s) : x(s), y(s), z(s) { }

    bool     operator== (const Vector3& b) const  { return x == b.x && y == b.y && z == b.z; }
    bool     operator!= (const Vector3& b) const  { return x != b.x || y != b.y || z != b.z; }
             
    Vector3  operator+  (const Vector3& b) const  { return Vector3(x + b.x, y + b.y, z + b.z); }
    Vector3& operator+= (const Vector3& b)        { x += b.x; y += b.y; z += b.z; return *this; }
    Vector3  operator-  (const Vector3& b) const  { return Vector3(x - b.x, y - b.y, z - b.z); }
    Vector3& operator-= (const Vector3& b)        { x -= b.x; y -= b.y; z -= b.z; return *this; }
    Vector3  operator- () const                   { return Vector3(-x, -y, -z); }

    // Scalar multiplication/division scales vector.
    Vector3  operator*  (T s) const               { return Vector3(x*s, y*s, z*s); }
    Vector3& operator*= (T s)                     { x *= s; y *= s; z *= s; return *this; }

    Vector3  operator/  (T s) const               { T rcp = T(1)/s;
                                                    return Vector3(x*rcp, y*rcp, z*rcp); }
    Vector3& operator/= (T s)                     { T rcp = T(1)/s;
                                                    x *= rcp; y *= rcp; z *= rcp;
                                                    return *this; }

    // Compare two vectors for equality with tolerance. Returns true if vectors match withing tolerance.
    bool      Compare(const Vector3&b, T tolerance = Mathf::Tolerance)
    {
        return (fabs(b.x-x) < tolerance) && (fabs(b.y-y) < tolerance) && (fabs(b.z-z) < tolerance);
    }
    
    // Dot product overload.
    // Used to calculate angle q between two vectors among other things,
    // as (A dot B) = |a||b|cos(q).
    T     operator*  (const Vector3& b) const    { return x*b.x + y*b.y + z*b.z; }

    // Compute cross product, which generates a normal vector.
    // Direction vector can be determined by right-hand rule: Pointing index finder in
    // direction a and middle finger in direction b, thumb will point in a.Cross(b).
    Vector3 Cross(const Vector3& b) const        { return Vector3(y*b.z - z*b.y,
                                                                  z*b.x - x*b.z,
                                                                  x*b.y - y*b.x); }

    // Returns the angle from this vector to b, in radians.
    T       Angle(const Vector3& b) const        { return acos((*this * b)/(Length()*b.Length())); }

    // Return Length of the vector squared.
    T       LengthSq() const                     { return (x * x + y * y + z * z); }
    // Return vector length.
    T       Length() const                       { return sqrt(LengthSq()); }

    // Returns distance between two points represented by vectors.
    T       Distance(Vector3& b) const           { return (*this - b).Length(); }
    
    // Determine if this a unit vector.
    bool    IsNormalized() const                 { return fabs(LengthSq() - T(1)) < Math<T>::Tolerance; }
    // Normalize, convention vector length to 1.    
    void    Normalize()                          { *this /= Length(); }
    // Returns normalized (unit) version of the vector without modifying itself.
    Vector3 Normalized() const                   { return *this / Length(); }

    // Linearly interpolates from this vector to another.
    // Factor should be between 0.0 and 1.0, with 0 giving full value to this.
    Vector3 Lerp(const Vector3& b, T f) const    { return *this*(T(1) - f) + b*f; }

    // Projects this vector onto the argument; in other words,
    // A.Project(B) returns projection of vector A onto B.
    Vector3 ProjectTo(const Vector3& b) const    { return b * ((*this * b) / b.LengthSq()); }
};


typedef Vector3<float>  Vector3f;
typedef Vector3<double> Vector3d;


//-------------------------------------------------------------------------------------
// ***** Matrix4f 

// Matrix4f is a 4x4 matrix used for 3d transformations and projections.
// Translation stored in the last column.
// The matrix is stored in row-major order in memory, meaning that values
// of the first row are stored before the next one.
//
// The arrangement of the matrix is chosen to be in Right-Handed 
// coordinate system and counterclockwise rotations when looking down
// the axis
//
// Transformation Order:
//   - Transformations are applied from right to left, so the expression
//     M1 * M2 * M3 * V means that the vector V is transformed by M3 first,
//     followed by M2 and M1. 
//
// Coordinate system: Right Handed
//
// Rotations: Counterclockwise when looking down the axis. All angles are in radians.
//    
//  | sx   01   02   tx |    // First column  (sx, 10, 20): Axis X basis vector.
//  | 10   sy   12   ty |    // Second column (01, sy, 21): Axis Y basis vector.
//  | 20   21   sz   tz |    // Third columnt (02, 12, sz): Axis Z basis vector.
//  | 30   31   32   33 |
//
//  The basis vectors are first three columns.

class Matrix4f
{
    static Matrix4f IdentityValue;

public:
    float M[4][4];    

    enum NoInitType { NoInit };

    // Construct with no memory initialization.
    Matrix4f(NoInitType) { }

    // By default, we construct identity matrix.
    Matrix4f()
    {
        SetIdentity();        
    }

    Matrix4f(float m11, float m12, float m13, float m14,
             float m21, float m22, float m23, float m24,
             float m31, float m32, float m33, float m34,
             float m41, float m42, float m43, float m44)
    {
        M[0][0] = m11; M[0][1] = m12; M[0][2] = m13; M[0][3] = m14;
        M[1][0] = m21; M[1][1] = m22; M[1][2] = m23; M[1][3] = m24;
        M[2][0] = m31; M[2][1] = m32; M[2][2] = m33; M[2][3] = m34;
        M[3][0] = m41; M[3][1] = m42; M[3][2] = m43; M[3][3] = m44;
    }

    Matrix4f(float m11, float m12, float m13,
             float m21, float m22, float m23,
             float m31, float m32, float m33)
    {
        M[0][0] = m11; M[0][1] = m12; M[0][2] = m13; M[0][3] = 0;
        M[1][0] = m21; M[1][1] = m22; M[1][2] = m23; M[1][3] = 0;
        M[2][0] = m31; M[2][1] = m32; M[2][2] = m33; M[2][3] = 0;
        M[3][0] = 0;   M[3][1] = 0;   M[3][2] = 0;   M[3][3] = 1;
    }

    static const Matrix4f& Identity()  { return IdentityValue; }

    void SetIdentity()
    {
        M[0][0] = M[1][1] = M[2][2] = M[3][3] = 1;
        M[0][1] = M[1][0] = M[2][3] = M[3][1] = 0;
        M[0][2] = M[1][2] = M[2][0] = M[3][2] = 0;
        M[0][3] = M[1][3] = M[2][1] = M[3][0] = 0;
    }

    // Multiplies two matrices into destination with minimum copying.
    static Matrix4f& Multiply(Matrix4f* d, const Matrix4f& a, const Matrix4f& b)
    {
        OVR_ASSERT((d != &a) && (d != &b));
        int i = 0;
        do {
            d->M[i][0] = a.M[i][0] * b.M[0][0] + a.M[i][1] * b.M[1][0] + a.M[i][2] * b.M[2][0] + a.M[i][3] * b.M[3][0];
            d->M[i][1] = a.M[i][0] * b.M[0][1] + a.M[i][1] * b.M[1][1] + a.M[i][2] * b.M[2][1] + a.M[i][3] * b.M[3][1];
            d->M[i][2] = a.M[i][0] * b.M[0][2] + a.M[i][1] * b.M[1][2] + a.M[i][2] * b.M[2][2] + a.M[i][3] * b.M[3][2];
            d->M[i][3] = a.M[i][0] * b.M[0][3] + a.M[i][1] * b.M[1][3] + a.M[i][2] * b.M[2][3] + a.M[i][3] * b.M[3][3];
        } while((++i) < 4);

        return *d;
    }

    Matrix4f operator* (const Matrix4f& b) const
    {
        Matrix4f result(Matrix4f::NoInit);
        Multiply(&result, *this, b);
        return result;
    }

    Matrix4f& operator*= (const Matrix4f& b)
    {
        return Multiply(this, Matrix4f(*this), b);
    }

    Matrix4f operator* (float s) const
    {
        return Matrix4f(M[0][0] * s, M[0][1] * s, M[0][2] * s, M[0][3] * s,
                        M[1][0] * s, M[1][1] * s, M[1][2] * s, M[1][3] * s,
                        M[2][0] * s, M[2][1] * s, M[2][2] * s, M[2][3] * s,
                        M[3][0] * s, M[3][1] * s, M[3][2] * s, M[3][3] * s);
    }

    Matrix4f& operator*= (float s)
    {
        M[0][0] *= s; M[0][1] *= s; M[0][2] *= s; M[0][3] *= s;
        M[1][0] *= s; M[1][1] *= s; M[1][2] *= s; M[1][3] *= s;
        M[2][0] *= s; M[2][1] *= s; M[2][2] *= s; M[2][3] *= s;
        M[3][0] *= s; M[3][1] *= s; M[3][2] *= s; M[3][3] *= s;
        return *this;
    }

    Vector3f Transform(const Vector3f& v) const
    {
        return Vector3f(M[0][0] * v.x + M[0][1] * v.y + M[0][2] * v.z + M[0][3],
                        M[1][0] * v.x + M[1][1] * v.y + M[1][2] * v.z + M[1][3],
                        M[2][0] * v.x + M[2][1] * v.y + M[2][2] * v.z + M[2][3]);
    }

    Matrix4f Transposed() const
    {
        return Matrix4f(M[0][0], M[1][0], M[2][0], M[3][0],
                        M[0][1], M[1][1], M[2][1], M[3][1],
                        M[0][2], M[1][2], M[2][2], M[3][2],
                        M[0][3], M[1][3], M[2][3], M[3][3]);
    }

    void     Transpose()
    {
        *this = Transposed();
    }


    float SubDet (const int* rows, const int* cols) const
    {
        return M[rows[0]][cols[0]] * (M[rows[1]][cols[1]] * M[rows[2]][cols[2]] - M[rows[1]][cols[2]] * M[rows[2]][cols[1]])
             - M[rows[0]][cols[1]] * (M[rows[1]][cols[0]] * M[rows[2]][cols[2]] - M[rows[1]][cols[2]] * M[rows[2]][cols[0]])
             + M[rows[0]][cols[2]] * (M[rows[1]][cols[0]] * M[rows[2]][cols[1]] - M[rows[1]][cols[1]] * M[rows[2]][cols[0]]);
    }

    float Cofactor(int I, int J) const
    {
        const int indices[4][3] = {{1,2,3},{0,2,3},{0,1,3},{0,1,2}};
        return ((I+J)&1) ? -SubDet(indices[I],indices[J]) : SubDet(indices[I],indices[J]);
    }

    float    Determinant() const
    {
        return M[0][0] * Cofactor(0,0) + M[0][1] * Cofactor(0,1) + M[0][2] * Cofactor(0,2) + M[0][3] * Cofactor(0,3);
    }

    Matrix4f Adjugated() const
    {
        return Matrix4f(Cofactor(0,0), Cofactor(1,0), Cofactor(2,0), Cofactor(3,0), 
                        Cofactor(0,1), Cofactor(1,1), Cofactor(2,1), Cofactor(3,1), 
                        Cofactor(0,2), Cofactor(1,2), Cofactor(2,2), Cofactor(3,2),
                        Cofactor(0,3), Cofactor(1,3), Cofactor(2,3), Cofactor(3,3));
    }

    Matrix4f Inverted() const
    {
        float det = Determinant();
        assert(det != 0);
        return Adjugated() * (1.0f/det);
    }

    void Invert()
    {
        *this = Inverted();
    }

    //AnnaSteve:
    // a,b,c, are the YawPitchRoll angles to be returned
    // rotation a around axis A1
    // is followed by rotation b around axis A2
    // is followed by rotation c around axis A3
    // rotations are CCW or CW (D) in LH or RH coordinate system (S)
    template <Axis A1, Axis A2, Axis A3, RotateDirection D, HandedSystem S>
    void ToEulerAngles(float *a, float *b, float *c)
    {
        OVR_COMPILER_ASSERT((A1 != A2) && (A2 != A3) && (A1 != A3));

        float psign = -1.0f;
        if (((A1 + 1) % 3 == A2) && ((A2 + 1) % 3 == A3)) // Determine whether even permutation
        psign = 1.0f;
        
        float pm = psign*M[A1][A3];
        if (pm < -1.0f + Math<float>::SingularityRadius)
        { // South pole singularity
            *a = 0.0f;
            *b = -S*D*Math<float>::PiOver2;
            *c = S*D*atan2( psign*M[A2][A1], M[A2][A2] );
        }
        else if (pm > 1.0 - Math<float>::SingularityRadius)
        { // North pole singularity
            *a = 0.0f;
            *b = S*D*Math<float>::PiOver2;
            *c = S*D*atan2( psign*M[A2][A1], M[A2][A2] );
        }
        else
        { // Normal case (nonsingular)
            *a = S*D*atan2( -psign*M[A2][A3], M[A3][A3] );
            *b = S*D*asin(pm);
            *c = S*D*atan2( -psign*M[A1][A2], M[A1][A1] );
        }

        return;
    }

    //AnnaSteve:
    // a,b,c, are the YawPitchRoll angles to be returned
    // rotation a around axis A1
    // is followed by rotation b around axis A2
    // is followed by rotation c around axis A1
    // rotations are CCW or CW (D) in LH or RH coordinate system (S)
    template <Axis A1, Axis A2, RotateDirection D, HandedSystem S>
    void ToEulerAnglesABA(float *a, float *b, float *c)
    {        
         OVR_COMPILER_ASSERT(A1 != A2);
  
        // Determine the axis that was not supplied
        int m = 3 - A1 - A2;

        float psign = -1.0f;
        if ((A1 + 1) % 3 == A2) // Determine whether even permutation
            psign = 1.0f;

        float c2 = M[A1][A1];
        if (c2 < -1.0 + Math<float>::SingularityRadius)
        { // South pole singularity
            *a = 0.0f;
            *b = S*D*Math<float>::Pi;
            *c = S*D*atan2( -psign*M[A2][m],M[A2][A2]);
        }
        else if (c2 > 1.0 - Math<float>::SingularityRadius)
        { // North pole singularity
            *a = 0.0f;
            *b = 0.0f;
            *c = S*D*atan2( -psign*M[A2][m],M[A2][A2]);
        }
        else
        { // Normal case (nonsingular)
            *a = S*D*atan2( M[A2][A1],-psign*M[m][A1]);
            *b = S*D*acos(c2);
            *c = S*D*atan2( M[A1][A2],psign*M[A1][m]);
        }
        return;
    }
  
    // Creates a matrix that converts the vertices from one coordinate system
    // to another.
    // 
    static Matrix4f AxisConversion(const WorldAxes& to, const WorldAxes& from)
    {        
        // Holds axis values from the 'to' structure
        int toArray[3] = { to.XAxis, to.YAxis, to.ZAxis };

        // The inverse of the toArray
        int inv[4]; 
        inv[0] = inv[abs(to.XAxis)] = 0;
        inv[abs(to.YAxis)] = 1;
        inv[abs(to.ZAxis)] = 2;

        Matrix4f m(0,  0,  0, 
                   0,  0,  0,
                   0,  0,  0);

        // Only three values in the matrix need to be changed to 1 or -1.
        m.M[inv[abs(from.XAxis)]][0] = float(from.XAxis/toArray[inv[abs(from.XAxis)]]);
        m.M[inv[abs(from.YAxis)]][1] = float(from.YAxis/toArray[inv[abs(from.YAxis)]]);
        m.M[inv[abs(from.ZAxis)]][2] = float(from.ZAxis/toArray[inv[abs(from.ZAxis)]]);
        return m;
    } 



    static Matrix4f Translation(const Vector3f& v)
    {
        Matrix4f t;
        t.M[0][3] = v.x;
        t.M[1][3] = v.y;
        t.M[2][3] = v.z;
        return t;
    }

    static Matrix4f Translation(float x, float y, float z = 0.0f)
    {
        Matrix4f t;
        t.M[0][3] = x;
        t.M[1][3] = y;
        t.M[2][3] = z;
        return t;
    }

    static Matrix4f Scaling(const Vector3f& v)
    {
        Matrix4f t;
        t.M[0][0] = v.x;
        t.M[1][1] = v.y;
        t.M[2][2] = v.z;
        return t;
    }

    static Matrix4f Scaling(float x, float y, float z)
    {
        Matrix4f t;
        t.M[0][0] = x;
        t.M[1][1] = y;
        t.M[2][2] = z;
        return t;
    }

    static Matrix4f Scaling(float s)
    {
        Matrix4f t;
        t.M[0][0] = s;
        t.M[1][1] = s;
        t.M[2][2] = s;
        return t;
    }

  

    //AnnaSteve : Just for quick testing.  Not for final API.  Need to remove case.
    static Matrix4f RotationAxis(Axis A, float angle, RotateDirection d, HandedSystem s)
    {
        float sina = s * d *sin(angle);
        float cosa = cos(angle);
        
        switch(A)
        {
        case Axis_X:
            return Matrix4f(1,  0,     0, 
                            0,  cosa,  -sina,
                            0,  sina,  cosa);
        case Axis_Y:
            return Matrix4f(cosa,  0,   sina, 
                            0,     1,   0,
                            -sina, 0,   cosa);
        case Axis_Z:
            return Matrix4f(cosa,  -sina,  0, 
                            sina,  cosa,   0,
                            0,     0,      1);
        }
    }


    // Creates a rotation matrix rotating around the X axis by 'angle' radians.
    // Rotation direction is depends on the coordinate system:
    //  RHS (Oculus default): Positive angle values rotate Counter-clockwise (CCW),
    //                        while looking in the negative axis direction. This is the
    //                        same as looking down from positive axis values towards origin.
    //  LHS: Positive angle values rotate clock-wise (CW), while looking in the
    //       negative axis direction.
    static Matrix4f RotationX(float angle)
    {
        float sina = sin(angle);
        float cosa = cos(angle);
        return Matrix4f(1,  0,     0, 
                        0,  cosa,  -sina,
                        0,  sina,  cosa);
    }

    // Creates a rotation matrix rotating around the Y axis by 'angle' radians.
    // Rotation direction is depends on the coordinate system:
    //  RHS (Oculus default): Positive angle values rotate Counter-clockwise (CCW),
    //                        while looking in the negative axis direction. This is the
    //                        same as looking down from positive axis values towards origin.
    //  LHS: Positive angle values rotate clock-wise (CW), while looking in the
    //       negative axis direction.
    static Matrix4f RotationY(float angle)
    {
        float sina = sin(angle);
        float cosa = cos(angle);
        return Matrix4f(cosa,  0,   sina, 
                        0,     1,   0,
                        -sina, 0,   cosa);
    }

    // Creates a rotation matrix rotating around the Z axis by 'angle' radians.
    // Rotation direction is depends on the coordinate system:
    //  RHS (Oculus default): Positive angle values rotate Counter-clockwise (CCW),
    //                        while looking in the negative axis direction. This is the
    //                        same as looking down from positive axis values towards origin.
    //  LHS: Positive angle values rotate clock-wise (CW), while looking in the
    //       negative axis direction.
    static Matrix4f RotationZ(float angle)
    {
        float sina = sin(angle);
        float cosa = cos(angle);
        return Matrix4f(cosa,  -sina,  0, 
                        sina,  cosa,   0,
                        0,     0,      1);
    }


    // LookAtRH creates a View transformation matrix for right-handed coordinate system.
    // The resulting matrix points camera from 'eye' towards 'at' direction, with 'up'
    // specifying the up vector. The resulting matrix should be used with PerspectiveRH
    // projection.
    static Matrix4f LookAtRH(const Vector3f& eye, const Vector3f& at, const Vector3f& up);

    // LookAtLH creates a View transformation matrix for left-handed coordinate system.
    // The resulting matrix points camera from 'eye' towards 'at' direction, with 'up'
    // specifying the up vector. 
    static Matrix4f LookAtLH(const Vector3f& eye, const Vector3f& at, const Vector3f& up);
    
    
    // PerspectiveRH creates a right-handed perspective projection matrix that can be
    // used with the Oculus sample renderer. 
    //  yfov   - Specifies vertical field of view in radians.
    //  aspect - Screen aspect ration, which is usually width/height for square pixels.
    //           Note that xfov = yfov * aspect.
    //  znear  - Absolute value of near Z clipping clipping range.
    //  zfar   - Absolute value of far  Z clipping clipping range (larger then near).
    // Even though RHS usually looks in the direction of negative Z, positive values
    // are expected for znear and zfar.
    static Matrix4f PerspectiveRH(float yfov, float aspect, float znear, float zfar);
    
    
    // PerspectiveRH creates a left-handed perspective projection matrix that can be
    // used with the Oculus sample renderer. 
    //  yfov   - Specifies vertical field of view in radians.
    //  aspect - Screen aspect ration, which is usually width/height for square pixels.
    //           Note that xfov = yfov * aspect.
    //  znear  - Absolute value of near Z clipping clipping range.
    //  zfar   - Absolute value of far  Z clipping clipping range (larger then near).
    static Matrix4f PerspectiveLH(float yfov, float aspect, float znear, float zfar);


    static Matrix4f Ortho2D(float w, float h);
};


//-------------------------------------------------------------------------------------
// ***** Quat

// Quatf represents a quaternion class used for rotations.
// 
// Quaternion multiplications are done in right-to-left order, to match the
// behavior of matrices.


template<class T>
class Quat
{
public:
    // w + Xi + Yj + Zk
    T x, y, z, w;    

    Quat() : x(0), y(0), z(0), w(1) {}
    Quat(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}


    // Constructs rotation quaternion around the axis.
    Quat(const Vector3<T>& axis, T angle)
    {
        Vector3<T> unitAxis = axis.Normalized();
        T          sinHalfAngle = sin(angle * T(0.5));

        w = cos(angle * T(0.5));
        x = unitAxis.x * sinHalfAngle;
        y = unitAxis.y * sinHalfAngle;
        z = unitAxis.z * sinHalfAngle;
    }

    //AnnaSteve:
    void AxisAngle(Axis A, T angle, RotateDirection d, HandedSystem s)
    {
        T sinHalfAngle = s * d *sin(angle * (T)0.5);
        T v[3];
        v[0] = v[1] = v[2] = (T)0;
        v[A] = sinHalfAngle;
        //return Quat(v[0], v[1], v[2], cos(angle * (T)0.5));
        w = cos(angle * (T)0.5);
        x = v[0];
        y = v[1];
        z = v[2];
    }


    void GetAxisAngle(Vector3<T>* axis, T* angle) const
    {
        if (LengthSq() > Math<T>::Tolerance * Math<T>::Tolerance)
        {
            *axis  = Vector3<T>(x, y, z).Normalized();
            *angle = 2 * acos(w);
        }
        else
        {
            *axis = Vector3<T>(1, 0, 0);
            *angle= 0;
        }
    }

    bool operator== (const Quat& b) const   { return x == b.x && y == b.y && z == b.z && w == b.w; }
    bool operator!= (const Quat& b) const   { return x != b.x || y != b.y || z != b.z || w != b.w; }

    Quat  operator+  (const Quat& b) const  { return Quat(x + b.x, y + b.y, z + b.z, w + b.w); }
    Quat& operator+= (const Quat& b)        { w += b.w; x += b.x; y += b.y; z += b.z; return *this; }
    Quat  operator-  (const Quat& b) const  { return Quat(x - b.x, y - b.y, z - b.z, w - b.w); }
    Quat& operator-= (const Quat& b)        { w -= b.w; x -= b.x; y -= b.y; z -= b.z; return *this; }

    Quat  operator*  (T s) const            { return Quat(x * s, y * s, z * s, w * s); }
    Quat& operator*= (T s)                  { w *= s; x *= s; y *= s; z *= s; return *this; }
    Quat  operator/  (T s) const            { T rcp = T(1)/s; return Quat(x * rcp, y * rcp, z * rcp, w *rcp); }
    Quat& operator/= (T s)                  { T rcp = T(1)/s; w *= rcp; x *= rcp; y *= rcp; z *= rcp; return *this; }

    // Get Imaginary part vector
    Vector3<T> Imag() const                 { return Vector3<T>(x,y,z); }

    // Get quaternion length.
    T       Length() const                  { return sqrt(x * x + y * y + z * z + w * w); }
    // Get quaternion length squared.
    T       LengthSq() const                { return (x * x + y * y + z * z + w * w); }
    // Simple Eulidean distance in R^4 (not SLERP distance, but at least respects Haar measure)
    T       Distance(const Quat& q) const
    {
        T d1 = (*this - q).Length();
        T d2 = (*this + q).Length(); // Antipoldal point check
        return (d1 < d2) ? d1 : d2;
    }
    T       DistanceSq(const Quat& q) const
    {
        T d1 = (*this - q).LengthSq();
        T d2 = (*this + q).LengthSq(); // Antipoldal point check
        return (d1 < d2) ? d1 : d2;
    }

    // Normalize
    bool    IsNormalized() const            { return fabs(LengthSq() - 1) < Math<T>::Tolerance; }
    void    Normalize()                     { *this /= Length(); }
    Quat    Normalized() const              { return *this / Length(); }

    // Returns conjugate of the quaternion. Produces inverse rotation if quaternion is normalized.
    Quat    Conj() const                    { return Quat(-x, -y, -z, w); }

    // AnnaSteve fixed: order of quaternion multiplication
    // Quaternion multiplication. Combines quaternion rotations, performing the one on the 
    // right hand side first.
    Quat  operator* (const Quat& b) const   { return Quat(w * b.x + x * b.w + y * b.z - z * b.y,
                                                          w * b.y - x * b.z + y * b.w + z * b.x,
                                                          w * b.z + x * b.y - y * b.x + z * b.w,
                                                          w * b.w - x * b.x - y * b.y - z * b.z); }

    // 
    // this^p normalized; same as rotating by this p times.
    Quat PowNormalized(T p) const
    {
        Vector3<T> v;
        T          a;
        GetAxisAngle(&v, &a);
        return Quat(v, a * p);
    }
    
    // Rotate transforms vector in a manner that matches Matrix rotations (counter-clockwise,
    // assuming negative direction of the axis). Standard formula: q(t) * V * q(t)^-1. 
    Vector3<T> Rotate(const Vector3<T>& v) const
    {
        return ((*this * Quat<T>(v.x, v.y, v.z, 0)) * Inverted()).Imag();
    }

    
    // Inversed quaternion rotates in the opposite direction.
    Quat        Inverted() const
    {
        return Quat(-x, -y, -z, w);
    }

    // Sets this quaternion to the one rotates in the opposite direction.
    void        Invert()
    {
        *this = Quat(-x, -y, -z, w);
    }
    
    // Converting quaternion to matrix.
    operator Matrix4f() const
    {
        T ww = w*w;
        T xx = x*x;
        T yy = y*y;
        T zz = z*z;

        return Matrix4f(float(ww + xx - yy - zz),  float(T(2) * (x*y - w*z)), float(T(2) * (x*z + w*y)),
                        float(T(2) * (x*y + w*z)), float(ww - xx + yy - zz),  float(T(2) * (y*z - w*x)),
                        float(T(2) * (x*z - w*y)), float(T(2) * (y*z + w*x)), float(ww - xx - yy + zz) );
    }

    
    // GetEulerAngles extracts Euler angles from the quaternion, in the specified order of
    // axis rotations and the specified coordinate system. Right-handed coordinate system
    // is the default, with CCW rotations while looking in the negative axis direction.
    // Here a,b,c, are the Yaw/Pitch/Roll angles to be returned.
    // rotation a around axis A1
    // is followed by rotation b around axis A2
    // is followed by rotation c around axis A3
    // rotations are CCW or CW (D) in LH or RH coordinate system (S)
    template <Axis A1, Axis A2, Axis A3, RotateDirection D, HandedSystem S>
    void GetEulerAngles(T *a, T *b, T *c)
    {
        OVR_COMPILER_ASSERT((A1 != A2) && (A2 != A3) && (A1 != A3));

        T Q[3] = { x, y, z };  //Quaternion components x,y,z

        T ww  = w*w;
        T Q11 = Q[A1]*Q[A1];
        T Q22 = Q[A2]*Q[A2];
        T Q33 = Q[A3]*Q[A3];

        T psign = T(-1.0);
        // Determine whether even permutation
        if (((A1 + 1) % 3 == A2) && ((A2 + 1) % 3 == A3))
            psign = T(1.0);
        
        T s2 = psign * T(2.0) * (psign*w*Q[A2] + Q[A1]*Q[A3]);

        if (s2 < (T)-1.0 + Math<T>::SingularityRadius)
        { // South pole singularity
            *a = T(0.0);
            *b = -S*D*Math<T>::PiOver2;
            *c = S*D*atan2((T)2.0*(psign*Q[A1]*Q[A2] + w*Q[A3]),
		                   ww + Q22 - Q11 - Q33 );
        }
        else if (s2 > (T)1.0 - Math<T>::SingularityRadius)
        {  // North pole singularity
            *a = (T)0.0;
            *b = S*D*Math<T>::PiOver2;
            *c = S*D*atan2((T)2.0*(psign*Q[A1]*Q[A2] + w*Q[A3]),
		                   ww + Q22 - Q11 - Q33);
        }
        else
        {
            *a = -S*D*atan2((T)-2.0*(w*Q[A1] - psign*Q[A2]*Q[A3]),
		                    ww + Q33 - Q11 - Q22);
            *b = S*D*asin(s2);
            *c = S*D*atan2((T)2.0*(w*Q[A3] - psign*Q[A1]*Q[A2]),
		                   ww + Q11 - Q22 - Q33);
        }      
        return;
    }

    template <Axis A1, Axis A2, Axis A3, RotateDirection D>
    void GetEulerAngles(T *a, T *b, T *c)
    { GetEulerAngles<A1, A2, A3, D, Handed_R>(a, b, c); }

    template <Axis A1, Axis A2, Axis A3>
    void GetEulerAngles(T *a, T *b, T *c)
    { GetEulerAngles<A1, A2, A3, Rotate_CCW, Handed_R>(a, b, c); }


    // GetEulerAnglesABA extracts Euler angles from the quaternion, in the specified order of
    // axis rotations and the specified coordinate system. Right-handed coordinate system
    // is the default, with CCW rotations while looking in the negative axis direction.
    // Here a,b,c, are the Yaw/Pitch/Roll angles to be returned.
    // rotation a around axis A1
    // is followed by rotation b around axis A2
    // is followed by rotation c around axis A1
    // Rotations are CCW or CW (D) in LH or RH coordinate system (S)
    template <Axis A1, Axis A2, RotateDirection D, HandedSystem S>
    void GetEulerAnglesABA(T *a, T *b, T *c)
    {
        OVR_COMPILER_ASSERT(A1 != A2);

        T Q[3] = {x, y, z}; // Quaternion components

        // Determine the missing axis that was not supplied
        int m = 3 - A1 - A2;

        T ww = w*w;
        T Q11 = Q[A1]*Q[A1];
        T Q22 = Q[A2]*Q[A2];
        T Qmm = Q[m]*Q[m];

        T psign = T(-1.0);
        if ((A1 + 1) % 3 == A2) // Determine whether even permutation
        {
            psign = (T)1.0;
        }

        T c2 = ww + Q11 - Q22 - Qmm;
        if (c2 < (T)-1.0 + Math<T>::SingularityRadius)
        { // South pole singularity
            *a = (T)0.0;
            *b = S*D*Math<T>::Pi;
            *c = S*D*atan2( (T)2.0*(w*Q[A1] - psign*Q[A2]*Q[m]),
		                    ww + Q22 - Q11 - Qmm);
        }
        else if (c2 > (T)1.0 - Math<T>::SingularityRadius)
        {  // North pole singularity
            *a = (T)0.0;
            *b = (T)0.0;
            *c = S*D*atan2( (T)2.0*(w*Q[A1] - psign*Q[A2]*Q[m]),
		                   ww + Q22 - Q11 - Qmm);
        }
        else
        {
            *a = S*D*atan2( psign*w*Q[m] + Q[A1]*Q[A2],
		                   w*Q[A2] -psign*Q[A1]*Q[m]);
            *b = S*D*acos(c2);
            *c = S*D*atan2( -psign*w*Q[m] + Q[A1]*Q[A2],
		                   w*Q[A2] + psign*Q[A1]*Q[m]);
        }
        return;
    }
};


typedef Quat<float>  Quatf;
typedef Quat<double> Quatd;



//-------------------------------------------------------------------------------------
// ***** Angle

// Cleanly representing the algebra of 2D rotations.
// The operations maintain the angle between -Pi and Pi, the same range as atan2.
// 

template<class T>
class Angle
{
public:
	enum AngularUnits
	{
		Radians = 0,
		Degrees = 1
	};

    Angle() : a(0) {}
    
	// Fix the range to be between -Pi and Pi
	Angle(T a_, AngularUnits u = Radians) : a((u == Radians) ? a_ : a_*Math<T>::DegreeToRadFactor) { FixRange(); }

	T    Get(AngularUnits u = Radians) const       { return (u == Radians) ? a : a*Math<T>::RadToDegreeFactor; }
	void Set(const T& x, AngularUnits u = Radians) { a = (u == Radians) ? x : x*Math<T>::DegreeToRadFactor; FixRange(); }
	int Sign() const                               { if (a == 0) return 0; else return (a > 0) ? 1 : -1; }
	T   Abs() const                                { return (a > 0) ? a : -a; }

    bool operator== (const Angle& b) const    { return a == b.a; }
    bool operator!= (const Angle& b) const    { return a != b.a; }
//	bool operator<  (const Angle& b) const    { return a < a.b; } 
//	bool operator>  (const Angle& b) const    { return a > a.b; } 
//	bool operator<= (const Angle& b) const    { return a <= a.b; } 
//	bool operator>= (const Angle& b) const    { return a >= a.b; } 
//	bool operator= (const T& x)               { a = x; FixRange(); }

	// These operations assume a is already between -Pi and Pi.
    Angle  operator+  (const Angle& b) const  { return Angle(a + b.a); }
	Angle  operator+  (const T& x) const      { return Angle(a + x); }
	Angle& operator+= (const Angle& b)        { a = a + b.a; FastFixRange(); return *this; }
	Angle& operator+= (const T& x)            { a = a + x; FixRange(); return *this; }
	Angle  operator-  (const Angle& b) const  { return Angle(a - b.a); }
	Angle  operator-  (const T& x) const      { return Angle(a - x); }
	Angle& operator-= (const Angle& b)        { a = a - b.a; FastFixRange(); return *this; }
	Angle& operator-= (const T& x)            { a = a - x; FixRange(); return *this; }
	
	T   Distance(const Angle& b)              { T c = fabs(a - b.a); return (c <= Math<T>::Pi) ? c : Math<T>::TwoPi - c; }

private:

	// The stored angle, which should be maintained between -Pi and Pi
	T a;

	// Fixes the angle range to [-Pi,Pi], but assumes no more than 2Pi away on either side 
	inline void FastFixRange()
	{
		if (a < -Math<T>::Pi)
			a += Math<T>::TwoPi;
		else if (a > Math<T>::Pi)
			a -= Math<T>::TwoPi;
	}

	// Fixes the angle range to [-Pi,Pi] for any given range, but slower then the fast method
	inline void FixRange()
	{
		a = fmod(a,Math<T>::TwoPi);
		if (a < -Math<T>::Pi)
			a += Math<T>::TwoPi;
		else if (a > Math<T>::Pi)
			a -= Math<T>::TwoPi;
	}
};


typedef Angle<float>  Anglef;
typedef Angle<double> Angled;


//-------------------------------------------------------------------------------------
// ***** Plane

// Consists of a normal vector and distance from the origin where the plane is located.

template<class T>
class Plane : public RefCountBase<Plane<T> >
{
public:
    Vector3<T> N;
    T          D;

    Plane() : D(0) {}

    // Normals must already be normalized
    Plane(const Vector3<T>& n, T d) : N(n), D(d) {}
    Plane(T x, T y, T z, T d) : N(x,y,z), D(d) {}

    // construct from a point on the plane and the normal
    Plane(const Vector3<T>& p, const Vector3<T>& n) : N(n), D(-(p * n)) {}

    // Find the point to plane distance. The sign indicates what side of the plane the point is on (0 = point on plane).
    T TestSide(const Vector3<T>& p) const
    {
        return (N * p) + D;
    }

    Plane<T> Flipped() const
    {
        return Plane(-N, -D);
    }

    void Flip()
    {
        N = -N;
        D = -D;
    }

	bool operator==(const Plane<T>& rhs) const
	{
		return (this->D == rhs.D && this->N == rhs.N);
	}
};

typedef Plane<float> Planef;

}

#endif
