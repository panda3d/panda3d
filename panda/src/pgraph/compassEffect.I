// Filename: compassEffect.I
// Created by:  drose (16Jul02)
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
//     Function: CompassEffect::Constructor
//       Access: Private
//  Description: Use CompassEffect::make() to construct a new
//               CompassEffect object.
////////////////////////////////////////////////////////////////////
INLINE CompassEffect::
CompassEffect() {
  _properties = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::get_reference
//       Access: Published
//  Description: Returns the reference node from which the
//               CompassEffect inherits its transform.  If this is
//               empty, it means the root of the scene graph.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &CompassEffect::
get_reference() const {
  return _reference;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::get_properties
//       Access: Published
//  Description: 

//               Returns the bitmask of properties that this
//               CompassEffect object inherits from its reference node
//               (or from the root).
////////////////////////////////////////////////////////////////////
INLINE int CompassEffect::
get_properties() const {
  return _properties;
}
