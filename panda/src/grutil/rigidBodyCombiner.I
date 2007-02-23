// Filename: rigidBodyCombiner.I
// Created by:  drose (22Feb07)
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
//     Function: RigidBodyCombiner::VDUnifier::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE RigidBodyCombiner::VDUnifier::
VDUnifier(const VertexTransform *transform, const GeomVertexData *orig) :
  _transform(transform),
  _orig(orig)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RigidBodyCombiner::VDUnifier::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool RigidBodyCombiner::VDUnifier::
operator < (const RigidBodyCombiner::VDUnifier &other) const {
  if (_transform != other._transform) {
    return _transform < other._transform;
  }
  return _orig < other._orig;
}
