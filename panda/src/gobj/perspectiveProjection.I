// Filename: perspectiveProjection.I
// Created by:  drose (18Feb99)
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
//     Function: PerspectiveProjection::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PerspectiveProjection::
PerspectiveProjection(const Frustumf &frustum) : _frustum(frustum) {
}


////////////////////////////////////////////////////////////////////
//     Function: PerspectiveProjection::get_frustum
//       Access: Public
//  Description: Returns the frustum that defines the perspective
//               projection.
////////////////////////////////////////////////////////////////////
INLINE const Frustumf &PerspectiveProjection::
get_frustum() const {
  return _frustum;
}

INLINE void PerspectiveProjection::
set_frustum(const Frustumf &frust) {
  _frustum = frust;
}

