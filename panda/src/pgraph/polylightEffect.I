// Filename: polylightEffect.I
// Created by:  sshodhan (02Jun04)
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
//     Function: PolylightEffect::Constructor
//       Access: Private
//  Description: Use PolylightEffect::make() to construct a new
//               PolylightEffect object.
////////////////////////////////////////////////////////////////////
INLINE PolylightEffect::
PolylightEffect() {
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::get_weight
//       Access: Published
//  Description: Get the weight value
////////////////////////////////////////////////////////////////////
INLINE float PolylightEffect:: 
get_weight() const {
  return _weight;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::get_contrib
//       Access: Published
//  Description: Returns CALL or CPROXIMAL
////////////////////////////////////////////////////////////////////
INLINE PolylightEffect::Contrib_Type PolylightEffect::
get_contrib() const {
  return _contribution_type;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::get_effect_center
//       Access: Published
//  Description: Return the value of the _effect_center
////////////////////////////////////////////////////////////////////
INLINE LPoint3f PolylightEffect::
get_effect_center() const {
  return _effect_center;
}
