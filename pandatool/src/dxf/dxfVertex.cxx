// Filename: dxfVertex.cxx
// Created by:  drose (04May04)
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

#include "dxfVertex.h"

////////////////////////////////////////////////////////////////////
//     Function: DXFVertex::Ordering operator
//       Access: Public
//  Description: This defines a unique ordering for vertices so that
//               the DXFVertexMap can group identical vertices
//               together.
////////////////////////////////////////////////////////////////////
int DXFVertex::
operator < (const DXFVertex &other) const {
  if (fabs(_p[0] - other._p[0]) > 0.0001) {
    return _p[0] < other._p[0];
  } else if (fabs(_p[1] - other._p[1]) > 0.0001) {
    return _p[1] < other._p[1];
  } else if (fabs(_p[2] - other._p[2]) > 0.0001) {
    return _p[2] < other._p[2];
  }

  return false;
}

