// Filename: cmath.I
// Created by:  drose (19May00)
// 
////////////////////////////////////////////////////////////////////

//Windows has isnan in a different place and with a different name
//than everyone else.  Sheesh
#ifdef _WIN32
#include <float.h>
#endif

INLINE float csqrt(float v) {
  return sqrtf(v);
}

INLINE float csin(float v) {
  return sinf(v);
}

INLINE float ccos(float v) {
  return cosf(v);
}

INLINE void
sincosf(float v, float *pSinResult, float *pCosResult) {

// msvc defines _M_IX86 for x86
#ifdef _M_IX86
#define fsincos __asm _emit 0xd9 __asm _emit 0xfb
    __asm {
        mov eax, pSinResult
        mov edx, pCosResult
        fld v
        fsincos
        fstp DWORD ptr [edx]
        fstp DWORD ptr [eax]
    }
#undef fsincos
#else //!_X86_
    *pSinResult = sinf(v);
    *pCosResult = cosf(v);
#endif //!_X86_
}

INLINE void
sincos(double v, double *pSinResult, double *pCosResult) {
#ifdef _M_IX86
#define sincos __asm _emit 0xd9 __asm _emit 0xfb
    __asm {
        mov eax, pSinResult
        mov edx, pCosResult
        fld v
        fsincos
        fstp QWORD ptr [edx]
        fstp QWORD ptr [eax]
    }
#undef sincos
#else //!_X86_
    *pSinResult = sin(v);
    *pCosResult = cos(v);
#endif //!_X86_
}

INLINE float cabs(float v) {
  return fabs(v);
}

INLINE float catan2(float y, float x) {
  return atan2f(y, x);
}

INLINE double csqrt(double v) {
  return sqrt(v);
}

INLINE double csin(double v) {
  return sin(v);
}

INLINE double ccos(double v) {
  return cos(v);
}

INLINE double cabs(double v) {
  return fabs(v);
}

INLINE double catan2(double y, double x) {
  return atan2(y, x);
}

INLINE bool cnan(double v) {
#ifndef _WIN32
  return (isnan(v) != 0);
#else
  return (_isnan(v) != 0);
#endif
}


