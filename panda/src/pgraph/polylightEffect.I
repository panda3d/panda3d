// Filename: polylightEffect.I
// Created by:  sshodhan (02Jun04)
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


////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::Constructor
//       Access: Protected
//  Description: Use PolylightEffect::make() to construct a new
//               PolylightEffect object.
////////////////////////////////////////////////////////////////////
INLINE PolylightEffect::
PolylightEffect() {
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::Constructor
//       Access: Protected
//  Description: Copy Constructor used by the const methods
//               to modify data on a copy and return a new one
////////////////////////////////////////////////////////////////////
INLINE PolylightEffect::
PolylightEffect(const PolylightEffect &copy) :
  _contribution_type(copy._contribution_type),
  _weight(copy._weight),
  _lightgroup(copy._lightgroup),
  _effect_center(copy._effect_center)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::get_weight
//       Access: Published
//  Description: Get the weight value
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PolylightEffect:: 
get_weight() const {
  return _weight;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::get_contrib
//       Access: Published
//  Description: Returns CT_all or CT_proximal
////////////////////////////////////////////////////////////////////
INLINE PolylightEffect::ContribType PolylightEffect::
get_contrib() const {
  return _contribution_type;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::get_effect_center
//       Access: Published
//  Description: Return the value of the _effect_center
////////////////////////////////////////////////////////////////////
INLINE LPoint3 PolylightEffect::
get_effect_center() const {
  return _effect_center;
}
