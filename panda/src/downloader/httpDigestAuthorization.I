// Filename: httpDigestAuthorization.I
// Created by:  drose (25Oct02)
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
//     Function: HTTPDigestAuthorization::hexdigit
//       Access: Private, Static
//  Description: Returns the ASCII character corresponding to the
//               hexadecimal representation of the indicated value,
//               which must be 0 <= value <= 15.
////////////////////////////////////////////////////////////////////
INLINE char HTTPDigestAuthorization::
hexdigit(int value) {
  return (value < 10) ? (value + '0') : (value - 10 + 'a');
}
