// Filename: dataGraphTraverser.I
// Created by:  drose (11Mar02)
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
//     Function: DataGraphTraverser::get_current_thread
//       Access: Public
//  Description: Returns the currently-executing thread object, as
//               passed to the DataGraphTraverser constructor.
////////////////////////////////////////////////////////////////////
INLINE Thread *DataGraphTraverser::
get_current_thread() const {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::CollectedData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DataGraphTraverser::CollectedData::
CollectedData() :
  _num_parents(0)
{
}
