// Filename: atomicAdjust.h
// Created by:  drose (09Aug02)
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

#ifndef ATOMICADJUST_H
#define ATOMICADJUST_H

#include "pandabase.h"
#include "atomicAdjustImpl.h"

////////////////////////////////////////////////////////////////////
//       Class : AtomicAdjust
// Description : A suite of functions to atomically adjust a numeric
//               value.  Some platforms require a bit more work than
//               others to guarantee that a multibyte value is changed
//               in one atomic operation.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS AtomicAdjust {
public:
  INLINE static int inc(int &var);
  INLINE static int dec(int &var);
  INLINE static int set(int &var, int new_value);
};

#include "atomicAdjust.I"

#endif
