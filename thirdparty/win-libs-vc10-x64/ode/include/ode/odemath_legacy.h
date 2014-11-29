/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#ifndef _ODE_ODEMATH_LEGACY_H_
#define _ODE_ODEMATH_LEGACY_H_


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
*	These macros are not used any more inside of ODE
*  They are kept for backward compatibility with external code that
*  might still be using them.
*/
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*
* General purpose vector operations with other vectors or constants.
*/

#define dOP(a,op,b,c) do { \
  (a)[0] = ((b)[0]) op ((c)[0]); \
  (a)[1] = ((b)[1]) op ((c)[1]); \
  (a)[2] = ((b)[2]) op ((c)[2]); \
} while (0)
#define dOPC(a,op,b,c) do { \
  (a)[0] = ((b)[0]) op (c); \
  (a)[1] = ((b)[1]) op (c); \
  (a)[2] = ((b)[2]) op (c); \
} while (0)
#define dOPE(a,op,b) do {\
  (a)[0] op ((b)[0]); \
  (a)[1] op ((b)[1]); \
  (a)[2] op ((b)[2]); \
} while (0)
#define dOPEC(a,op,c) do { \
  (a)[0] op (c); \
  (a)[1] op (c); \
  (a)[2] op (c); \
} while (0)

/// Define an equation with operators
/// For example this function can be used to replace
/// <PRE>
/// for (int i=0; i<3; ++i)
///   a[i] += b[i] + c[i];
/// </PRE>
#define dOPE2(a,op1,b,op2,c) do { \
  (a)[0] op1 ((b)[0]) op2 ((c)[0]); \
  (a)[1] op1 ((b)[1]) op2 ((c)[1]); \
  (a)[2] op1 ((b)[2]) op2 ((c)[2]); \
} while (0)


#define dLENGTHSQUARED(a) dCalcVectorLengthSquare3(a)
#define dLENGTH(a) dCalcVectorLength3(a)
#define dDISTANCE(a, b) dCalcPointsDistance3(a, b)


#define dDOT(a, b) dCalcVectorDot3(a, b)
#define dDOT13(a, b) dCalcVectorDot3_13(a, b)
#define dDOT31(a, b) dCalcVectorDot3_31(a, b)
#define dDOT33(a, b) dCalcVectorDot3_33(a, b)
#define dDOT14(a, b) dCalcVectorDot3_14(a, b)
#define dDOT41(a, b) dCalcVectorDot3_41(a, b)
#define dDOT44(a, b) dCalcVectorDot3_44(a, b)


/*
* cross product, set a = b x c. dCROSSpqr means that elements of `a', `b'
* and `c' are spaced p, q and r indexes apart respectively.
* dCROSS() means dCROSS111. `op' is normally `=', but you can set it to
* +=, -= etc to get other effects.
*/

#define dCROSS(a,op,b,c) \
  do { \
  (a)[0] op ((b)[1]*(c)[2] - (b)[2]*(c)[1]); \
  (a)[1] op ((b)[2]*(c)[0] - (b)[0]*(c)[2]); \
  (a)[2] op ((b)[0]*(c)[1] - (b)[1]*(c)[0]); \
  } while(0)
#define dCROSSpqr(a,op,b,c,p,q,r) \
  do { \
  (a)[  0] op ((b)[  q]*(c)[2*r] - (b)[2*q]*(c)[  r]); \
  (a)[  p] op ((b)[2*q]*(c)[  0] - (b)[  0]*(c)[2*r]); \
  (a)[2*p] op ((b)[  0]*(c)[  r] - (b)[  q]*(c)[  0]); \
  } while(0)
#define dCROSS114(a,op,b,c) dCROSSpqr(a,op,b,c,1,1,4)
#define dCROSS141(a,op,b,c) dCROSSpqr(a,op,b,c,1,4,1)
#define dCROSS144(a,op,b,c) dCROSSpqr(a,op,b,c,1,4,4)
#define dCROSS411(a,op,b,c) dCROSSpqr(a,op,b,c,4,1,1)
#define dCROSS414(a,op,b,c) dCROSSpqr(a,op,b,c,4,1,4)
#define dCROSS441(a,op,b,c) dCROSSpqr(a,op,b,c,4,4,1)
#define dCROSS444(a,op,b,c) dCROSSpqr(a,op,b,c,4,4,4)


/*
* set a 3x3 submatrix of A to a matrix such that submatrix(A)*b = a x b.
* A is stored by rows, and has `skip' elements per row. the matrix is
* assumed to be already zero, so this does not write zero elements!
* if (plus,minus) is (+,-) then a positive version will be written.
* if (plus,minus) is (-,+) then a negative version will be written.
*/

#define dCROSSMAT(A,a,skip,plus,minus) \
  do { \
  (A)[1] = minus (a)[2]; \
  (A)[2] = plus (a)[1]; \
  (A)[(skip)+0] = plus (a)[2]; \
  (A)[(skip)+2] = minus (a)[0]; \
  (A)[2*(skip)+0] = minus (a)[1]; \
  (A)[2*(skip)+1] = plus (a)[0]; \
  } while(0)




/* 
Note: NEVER call any of these functions/macros with the same variable for A and C, 
it is not equivalent to A*=B.
*/

#define dMULTIPLY0_331(A, B, C) dMultiply0_331(A, B, C)
#define dMULTIPLY1_331(A, B, C) dMultiply1_331(A, B, C)
#define dMULTIPLY0_133(A, B, C) dMultiply0_133(A, B, C)
#define dMULTIPLY0_333(A, B, C) dMultiply0_333(A, B, C)
#define dMULTIPLY1_333(A, B, C) dMultiply1_333(A, B, C)
#define dMULTIPLY2_333(A, B, C) dMultiply2_333(A, B, C)

#define dMULTIPLYADD0_331(A, B, C) dMultiplyAdd0_331(A, B, C)
#define dMULTIPLYADD1_331(A, B, C) dMultiplyAdd1_331(A, B, C)
#define dMULTIPLYADD0_133(A, B, C) dMultiplyAdd0_133(A, B, C)
#define dMULTIPLYADD0_333(A, B, C) dMultiplyAdd0_333(A, B, C)
#define dMULTIPLYADD1_333(A, B, C) dMultiplyAdd1_333(A, B, C)
#define dMULTIPLYADD2_333(A, B, C) dMultiplyAdd2_333(A, B, C)


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
*	These macros are not used any more inside of ODE
*  They are kept for backward compatibility with external code that
*  might still be using them.
*/
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


#endif // #ifndef _ODE_ODEMATH_LEGACY_H_
