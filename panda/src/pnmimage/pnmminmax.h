// Filename: pnmminmax.h
// Created by:  drose (23Aug96)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef _PNMMINMAX_H_
#define _PNMMINMAX_H_

#include <pandabase.h>

#include <algorithm>

// We now inherit these functions from STL.

/*
template <class Type>
inline Type
max(const Type &a, const Type &b) {
  return (a<b) ? b : a;
}

template <class Type>
inline Type
min(const Type &a, const Type &b) {
  return (a<b) ? a : b;
}
*/

template <class Type>
inline Type
bounds(const Type &value, const Type &lower, const Type &upper) {
  return min(max(value, lower), upper);
}

#endif
