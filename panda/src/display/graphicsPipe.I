// Filename: graphicsPipe.I
// Created by:  frang (07Mar99)
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
//     Function: GraphicsPipe::get_all_pipes
//       Access: Private, Static
//  Description: Returns a reference to the static _all_pipes
//               container.  This wrapper function allows us to
//               guarantee creation of the container before we attempt
//               to reference it.
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipe::Pipes &GraphicsPipe::
get_all_pipes() {
  if (_all_pipes == NULL) {
    _all_pipes = new Pipes;
  }
  return *_all_pipes;
}
