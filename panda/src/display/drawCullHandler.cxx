// Filename: drawCullHandler.cxx
// Created by:  drose (25Feb02)
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

#include "drawCullHandler.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "graphicsStateGuardian.h"


////////////////////////////////////////////////////////////////////
//     Function: DrawCullHandler::record_geom
//       Access: Public, Virtual
//  Description: This callback function is intended to be overridden
//               by a derived class.  This is called as each Geom is
//               discovered by the CullTraverser.
////////////////////////////////////////////////////////////////////
void DrawCullHandler::
record_geom(Geom *geom, const TransformState *transform,
            const RenderState *state) {
  _gsg->set_transform(transform);
  _gsg->set_state(state);
  geom->draw(_gsg);
}
