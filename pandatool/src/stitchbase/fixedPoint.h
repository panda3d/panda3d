// Filename: fixedPoint.h
// Created by:  drose (06Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

// Simple fixed-point arithmetic definitions, for support of
// TriangleRasterizer.  Totally ripped off from Mesa.

typedef int FixedPoint;

#define FIXED_ONE       0x00000800
#define FIXED_HALF      0x00000400
#define FIXED_FRAC_MASK 0x000007FF
#define FIXED_INT_MASK  (~FIXED_FRAC_MASK)
#define FIXED_EPSILON   1
#define FIXED_SCALE     2048.0
#define FIXED_SHIFT     11
#define FloatToFixed(X) ((FixedPoint) ((X) * FIXED_SCALE))
#define IntToFixed(I)   ((I) << FIXED_SHIFT)
#define FixedToInt(X)   ((X) >> FIXED_SHIFT)
#define FixedToUns(X)   (((unsigned int)(X)) >> 11)
#define FixedCeil(X)    (((X) + FIXED_ONE - FIXED_EPSILON) & FIXED_INT_MASK)
#define FixedFloor(X)   ((X) & FIXED_INT_MASK)
/* 0.00048828125 = 1/FIXED_SCALE */
#define FixedToFloat(X) ((X) * 0.00048828125)
#define PosFloatToFixed(X)      FloatToFixed(X)
#define SignedFloatToFixed(X)   FloatToFixed(X)

#endif
