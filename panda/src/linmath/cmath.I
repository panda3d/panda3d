// Filename: cmath.I
// Created by:  drose (19May00)
// 
////////////////////////////////////////////////////////////////////

//Windows has isnan in a different place and with a different name
//than everyone else.  Sheesh
#ifdef _WIN32
#include <float.h>
#endif

INLINE_LINMATH float csqrt(float v) {
  return sqrtf(v);
}

INLINE_LINMATH float csin(float v) {
  return sinf(v);
}

INLINE_LINMATH float ccos(float v) {
  return cosf(v);
}

INLINE_LINMATH void
sincosf(float v, float *pSinResult, float *pCosResult) {

// MS VC defines _M_IX86 for x86.  gcc should define _X86_
#if defined(_M_IX86) || defined(_X86_)
//#define fsincos_opcode __asm _emit 0xd9 __asm _emit 0xfb
    __asm {
        mov eax, pSinResult
        mov edx, pCosResult
        fld v
        fsincos
        fstp DWORD ptr [edx]
        fstp DWORD ptr [eax]
    }
#else //!_X86_
    *pSinResult = sinf(v);
    *pCosResult = cosf(v);
#endif //!_X86_
}

INLINE_LINMATH void
sincosd(double v, double *pSinResult, double *pCosResult) {
#if defined(_M_IX86) || defined(_X86_)
//#define fsincos_opcode __asm _emit 0xd9 __asm _emit 0xfb
    __asm {
        mov eax, pSinResult
        mov edx, pCosResult
        fld v
        fsincos
        fstp QWORD ptr [edx]
        fstp QWORD ptr [eax]
    }
#else //!_X86_
    *pSinResult = sin(v);
    *pCosResult = cos(v);
#endif //!_X86_
}

INLINE_LINMATH void csincos(float v,float *pSinResult, float *pCosResult) {
  sincosf(v,pSinResult,pCosResult);
}

INLINE_LINMATH void csincos(double v,double *pSinResult, double *pCosResult) {
  sincosd(v,pSinResult,pCosResult);
}

INLINE_LINMATH float cabs(float v) {
  return fabs(v);
}

INLINE_LINMATH float catan2(float y, float x) {
  return atan2f(y, x);
}

INLINE_LINMATH double csqrt(double v) {
  return sqrt(v);
}

INLINE_LINMATH double csin(double v) {
  return sin(v);
}

INLINE_LINMATH double ccos(double v) {
  return cos(v);
}

INLINE_LINMATH double cabs(double v) {
  return fabs(v);
}

INLINE_LINMATH double catan2(double y, double x) {
  return atan2(y, x);
}

INLINE_LINMATH bool cnan(double v) {
#ifndef _WIN32
  return (isnan(v) != 0);
#else
  return (_isnan(v) != 0);
#endif
}


