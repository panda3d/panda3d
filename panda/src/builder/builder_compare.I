// Filename: builder_compare.I
// Created by:  drose (31Jul01)
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

INLINE int
builder_compare(const LVecBase2f &a, const LVecBase2f &b) {
  return a.compare_to(b);
}

INLINE int
builder_compare(const LVecBase3f &a, const LVecBase3f &b) {
  return a.compare_to(b);
}

INLINE int
builder_compare(const LVecBase4f &a, const LVecBase4f &b) {
  return a.compare_to(b);
}

INLINE int
builder_compare(ushort a, ushort b) {
  return (int)a - (int)b;
}
