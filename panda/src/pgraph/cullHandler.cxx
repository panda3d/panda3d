// Filename: cullHandler.cxx
// Created by:  drose (23Feb02)
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

#include "cullHandler.h"
#include "cullableObject.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CullHandler::
CullHandler() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CullHandler::
~CullHandler() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::record_object
//       Access: Public, Virtual
//  Description: This callback function is intended to be overridden
//               by a derived class.  This is called as each Geom is
//               discovered by the CullTraverser.
//
//               The CullHandler becomes the owner of the
//               CullableObject pointer and is expected to delete it
//               later.
////////////////////////////////////////////////////////////////////
void CullHandler::
record_object(CullableObject *object, const CullTraverser *traverser) {
  nout << *object->_geom << " " << *object->_internal_transform << " "
       << *object->_state << "\n";
  delete object;
}

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::end_traverse
//       Access: Public, Virtual
//  Description: This callback function is intended to be overridden
//               by a derived class.  This is called at the end of the
//               traversal.
////////////////////////////////////////////////////////////////////
void CullHandler::
end_traverse() {
}
