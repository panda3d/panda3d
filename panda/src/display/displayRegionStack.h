// Filename: displayRegionStack.h
// Created by:  drose (06Oct99)
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

#ifndef DISPLAYREGIONSTACK_H
#define DISPLAYREGIONSTACK_H

#include "pandabase.h"

#include "displayRegion.h"

class GraphicsStateGuardian;

////////////////////////////////////////////////////////////////////
//       Class : DisplayRegionStack
// Description : An instance of this kind of object is returned by
//               GraphicsStateGuardian::push_display_region().  It
//               holds the information needed to restore the previous
//               display region in the subsequent matching call to
//               pop_display_region().
////////////////////////////////////////////////////////////////////
class DisplayRegionStack {
public:
  INLINE DisplayRegionStack();
  INLINE ~DisplayRegionStack();
  INLINE DisplayRegionStack(const DisplayRegionStack &copy);
  INLINE void operator =(const DisplayRegionStack &copy);

private:
  CPT(DisplayRegion) _display_region;
  int _stack_level;
  friend class GraphicsStateGuardian;
};

#include "displayRegionStack.I"

#endif
