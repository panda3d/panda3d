// Filename: texProjectorEffect.I
// Created by:  drose (25Jul04)
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
//     Function: TexProjectorEffect::Constructor
//       Access: Protected
//  Description: Use TexProjectorEffect::make() to construct a new
//               TexProjectorEffect object.
////////////////////////////////////////////////////////////////////
INLINE TexProjectorEffect::
TexProjectorEffect() {
}

////////////////////////////////////////////////////////////////////
//     Function: TexProjectorEffect::Copy Constructor
//       Access: Protected
//  Description: Use TexProjectorEffect::make() to construct a new
//               TexProjectorEffect object.
////////////////////////////////////////////////////////////////////
INLINE TexProjectorEffect::
TexProjectorEffect(const TexProjectorEffect &copy) :
  _stages(copy._stages)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: TexProjectorEffect::StageDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TexProjectorEffect::StageDef::
StageDef() :
  _to_lens_node(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TexProjectorEffect::StageDef::set_from
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TexProjectorEffect::StageDef::
set_from(const NodePath &from) {
  _from = from;
}

////////////////////////////////////////////////////////////////////
//     Function: TexProjectorEffect::StageDef::set_lens_index
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TexProjectorEffect::StageDef::
set_lens_index(int lens_index) {
  _lens_index = lens_index;
}

////////////////////////////////////////////////////////////////////
//     Function: TexProjectorEffect::StageDef::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int TexProjectorEffect::StageDef::
compare_to(const TexProjectorEffect::StageDef &other) const {
  int compare = _to.compare_to(other._to);
  if (compare != 0) {
    return compare;
  }

  if (_lens_index != other._lens_index) {
    return _lens_index - other._lens_index;
  }

  return _from.compare_to(other._from);
}
