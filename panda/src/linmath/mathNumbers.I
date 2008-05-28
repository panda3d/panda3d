// Filename: mathNumbers.I
// Created by:  drose (22Jan05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: MathNumbers::cpi
//       Access: Public, Static
//  Description: Returns pi as a single-precision or double-precision
//               number, according to the type of the parameter.
////////////////////////////////////////////////////////////////////
INLINE float MathNumbers::
cpi(float) {
  return pi_f;
}

////////////////////////////////////////////////////////////////////
//     Function: MathNumbers::cln2
//       Access: Public, Static
//  Description: Returns ln(2) as a single-precision or double-precision
//               number, according to the type of the parameter.
////////////////////////////////////////////////////////////////////
INLINE float MathNumbers::
cln2(float) {
  return ln2_f;
}

////////////////////////////////////////////////////////////////////
//     Function: MathNumbers::cpi
//       Access: Public, Static
//  Description: Returns pi as a single-precision or double-precision
//               number, according to the type of the parameter.
////////////////////////////////////////////////////////////////////
INLINE double MathNumbers::
cpi(double) {
  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: MathNumbers::cln2
//       Access: Public, Static
//  Description: Returns ln(2) as a single-precision or double-precision
//               number, according to the type of the parameter.
////////////////////////////////////////////////////////////////////
INLINE double MathNumbers::
cln2(double) {
  return ln2;
}
