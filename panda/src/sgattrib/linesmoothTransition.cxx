// Filename: linesmoothTransition.cxx
// Created by:  mike (08Feb99)
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

#include "linesmoothTransition.h"
#include "linesmoothAttribute.h"

TypeHandle LinesmoothTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LinesmoothTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated LinesmoothTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *LinesmoothTransition::
make_copy() const {
  return new LinesmoothTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LinesmoothTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated LinesmoothAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *LinesmoothTransition::
make_attrib() const {
  return new LinesmoothAttribute;
}
