// Filename: lensStack.h
// Created by:  drose (25Feb02)
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

#ifndef LENSSTACK_H
#define LENSSTACK_H

#include "pandabase.h"

#include "lens.h"

class GraphicsStateGuardian;

////////////////////////////////////////////////////////////////////
//       Class : LensStack
// Description : An instance of this kind of object is returned by
//               GraphicsStateGuardian::push_lens().  It holds the
//               information needed to restore the previous display
//               region in the subsequent matching call to pop_lens().
////////////////////////////////////////////////////////////////////
class LensStack {
public:
  INLINE LensStack();
  INLINE ~LensStack();
  INLINE LensStack(const LensStack &copy);
  INLINE void operator =(const LensStack &copy);

private:
  CPT(Lens) _lens;
  int _stack_level;
  friend class GraphicsStateGuardian;
};

#include "lensStack.I"

#endif
