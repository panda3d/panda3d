// Filename: collisionVisualizer.I
// Created by:  drose (17Apr03)
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
//     Function: CollisionVisualizer::SolidInfo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionVisualizer::SolidInfo::
SolidInfo() {
  _detected_count = 0;
  _missed_count = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::set_viz_scale
//       Access: Published
//  Description: Scales the objects that are drawn to represent the
//               normals and points of the collisions.  By default,
//               these objects are drawn at an arbitrary scale which
//               is appropriate if the scene units are measured in
//               feet.  Change this scale accordinatly if the scene
//               units are measured on some other scale or if you need
//               to observe these objects from farther away.
////////////////////////////////////////////////////////////////////
INLINE void CollisionVisualizer::
set_viz_scale(float viz_scale) {
  _viz_scale = viz_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::get_viz_scale
//       Access: Published
//  Description: Returns the value last set by set_viz_scale().
////////////////////////////////////////////////////////////////////
INLINE float CollisionVisualizer::
get_viz_scale() const {
  return _viz_scale;
}
