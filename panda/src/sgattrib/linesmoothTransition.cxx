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

PT(NodeTransition) LinesmoothTransition::_initial;
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
//     Function: LinesmoothTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated LinesmoothTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *LinesmoothTransition::
make_initial() const {
  if (_initial.is_null()) {
    _initial = new LinesmoothTransition;
  }
  return _initial;
}

////////////////////////////////////////////////////////////////////
//     Function: LinesmoothTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void LinesmoothTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_linesmooth(this);
}
