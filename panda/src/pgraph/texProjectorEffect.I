// Filename: texProjectorEffect.I
// Created by:  drose (25Jul04)
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

  return _from.compare_to(other._from);
}
