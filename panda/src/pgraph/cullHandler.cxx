// Filename: cullHandler.cxx
// Created by:  drose (23Feb02)
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

#include "cullHandler.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::record_geom
//       Access: Public, Virtual
//  Description: This callback function is intended to be overridden
//               by a derived class.  This is called as each Geom is
//               discovered by the CullTraverser.
//
//               This default method simply outputs a message to cerr;
//               it's not intended to be used except for debugging.
////////////////////////////////////////////////////////////////////
void CullHandler::
record_geom(Geom *geom, const TransformState *transform,
            const RenderState *state) {
  cerr << *geom << " " << *transform << " " << *state << "\n";
}
