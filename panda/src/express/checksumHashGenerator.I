// Filename: checksumHashGenerator.I
// Created by:  drose (14May01)
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
//     Function: ChecksumHashGenerator::add_int
//       Access: Public
//  Description: Adds another integer to the hash so far.  This
//               function should be overridden in base classes; this
//               is the principle implementation of the HashGenerator.
////////////////////////////////////////////////////////////////////
INLINE void ChecksumHashGenerator::
add_int(int sum) {
  _hash += (size_t)sum;
}

////////////////////////////////////////////////////////////////////
//     Function: ChecksumHashGenerator::add_bool
//       Access: Public
//  Description: Adds a boolean flag.
////////////////////////////////////////////////////////////////////
INLINE void ChecksumHashGenerator::
add_bool(bool flag) {
  add_int(flag);
}

////////////////////////////////////////////////////////////////////
//     Function: ChecksumHashGenerator::add_fp
//       Access: Public
//  Description: Adds a floating-point number, first converting it to
//               fixed point by dividing it by the indicated
//               threshold.
////////////////////////////////////////////////////////////////////
INLINE void ChecksumHashGenerator::
add_fp(float number, float threshold) {
  add_int((int)(number / threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: ChecksumHashGenerator::add_fp
//       Access: Public
//  Description: Adds a floating-point number, first converting it to
//               fixed point by dividing it by the indicated
//               threshold.
////////////////////////////////////////////////////////////////////
INLINE void ChecksumHashGenerator::
add_fp(double number, double threshold) {
  add_int((int)(number / threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: ChecksumHashGenerator::add_pointer
//       Access: Public
//  Description: Adds a pointer, derived simply by casting the pointer
//               to an integer.  This should be good enough even on
//               architectures for which this cast is lossy.
////////////////////////////////////////////////////////////////////
INLINE void ChecksumHashGenerator::
add_pointer(void *ptr) {
  add_int((int)ptr);
}
