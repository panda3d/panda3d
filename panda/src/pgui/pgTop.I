// Filename: pgTop.I
// Created by:  drose (02Jul01)
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


////////////////////////////////////////////////////////////////////
//     Function: PGTop::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGTop::
PGTop(const PGTop &copy) :
  NamedNode(copy),
  _watcher(copy._watcher)
{
  _gsg = (GraphicsStateGuardian *)NULL;
  _trav = (RenderTraverser *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PGTop::
operator = (const PGTop &copy) {
  NamedNode::operator = (copy);
  _watcher = copy._watcher;
}


////////////////////////////////////////////////////////////////////
//     Function: PGTop::get_mouse_watcher
//       Access: Published
//  Description: Returns the MouseWatcher pointer that the PGTop object
//               registers its PG items with, or NULL if the
//               MouseWatcher has not yet been set.
////////////////////////////////////////////////////////////////////
INLINE MouseWatcher *PGTop::
get_mouse_watcher() const {
  return _watcher;
}
