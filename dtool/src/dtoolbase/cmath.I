// Filename: cmath.I
// Created by:  drose (19May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

//Windows has isnan in a different place and with a different name
//than everyone else.  Sheesh
#ifdef _WIN32
#include <float.h>
#endif

#ifdef __APPLE__
#define      isnan( x )         ( ( sizeof ( x ) == sizeof(double) ) ?           \
                              __isnand ( x ) :                                 \
                                ( sizeof ( x ) == sizeof( float) ) ?            \
                              __isnanf ( x ) :                                 \
                              __isnan  ( x ) )
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

INLINE float ctan(float v) {
  return tanf(v);
}

INLINE void
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

INLINE void
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

INLINE void csincos(float v,float *pSinResult, float *pCosResult) {
  sincosf(v,pSinResult,pCosResult);
}

INLINE void csincos(double v,double *pSinResult, double *pCosResult) {
  sincosd(v,pSinResult,pCosResult);
}

INLINE float cabs(float v) {
  return fabs(v);
}

INLINE float catan(float v) {
  return atanf(v);
}

INLINE float catan2(float y, float x) {
  return atan2f(y, x);
}

#ifdef __INTEL_COMPILER
// see float.h
#define FPU_CONTROLWORD_WRITEMASK    0xFFFFF        // if you look at defn of _CW_DEFAULT, all settings fall within 0xFFFFF
#define FPU_CONTROLWORD_NEW_SETTING  _CW_DEFAULT
#endif  

INLINE double cfloor(double f) {
  #ifdef __INTEL_COMPILER
    // intel floor doesnt work right if fpu mode is not double, so make double-prec mode is on
    unsigned int saved_fpu_control_word=_controlfp(0x0,0x0);
    _controlfp(FPU_CONTROLWORD_NEW_SETTING,FPU_CONTROLWORD_WRITEMASK);
    double retval=floor(f);
    _controlfp(saved_fpu_control_word,FPU_CONTROLWORD_WRITEMASK);
    return retval;
  #else
    return floor(f);  
  #endif
}

INLINE double cceil(double f) {
  #ifdef __INTEL_COMPILER
    // intel ceil doesnt work right if fpu mode is not double, so make double-prec mode is on
    unsigned int saved_fpu_control_word=_controlfp(0x0,0x0);
    _controlfp(FPU_CONTROLWORD_NEW_SETTING,FPU_CONTROLWORD_WRITEMASK);
    double retval=ceil(f);
    _controlfp(saved_fpu_control_word,FPU_CONTROLWORD_WRITEMASK);
    return retval;
  #else
    return ceil(f);  
  #endif
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

INLINE double ctan(double v) {
  return tan(v);
}

INLINE double cabs(double v) {
  return fabs(v);
}

INLINE double catan(double v) {
  return atan(v);
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


