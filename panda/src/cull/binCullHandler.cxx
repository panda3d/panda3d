// Filename: binCullHandler.cxx
// Created by:  drose (28Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "binCullHandler.h"
#include "pStatTimer.h"

////////////////////////////////////////////////////////////////////
//     Function: BinCullHandler::record_object
//       Access: Public, Virtual
//  Description: This callback function is intended to be overridden
//               by a derived class.  This is called as each Geom is
//               discovered by the CullTraverser.
////////////////////////////////////////////////////////////////////
void BinCullHandler::
record_object(CullableObject *object, const CullTraverser *traverser) {
  _cull_result->add_object(object, traverser);
}
