// Filename: mathNumbers.I
// Created by:  drose (22Jan05)
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
