// Filename: eggAttributes.I
// Created by:  drose (16Jan99)
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
//     Function: EggAttributes::has_normal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggAttributes::
has_normal() const {
  return (_flags & F_has_normal) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAttributes::get_normal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Normald &EggAttributes::
get_normal() const {
  nassertr(has_normal(), _normal);
  return _normal;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAttributes::set_normal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggAttributes::
set_normal(const Normald &normal) {
  _normal = normal;
  _flags |= F_has_normal;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAttributes::clear_normal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggAttributes::
clear_normal() {
  _flags &= ~F_has_normal;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAttributes::has_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggAttributes::
has_color() const {
  return (_flags & F_has_color) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAttributes::get_color
//       Access: Public
//  Description: Returns the color set on this particular attribute.
//               If there is no color set, returns white.
////////////////////////////////////////////////////////////////////
INLINE Colorf EggAttributes::
get_color() const {
  if (has_color()) {
    return _color;
  } else {
    return Colorf(1.0, 1.0, 1.0, 1.0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggAttributes::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggAttributes::
set_color(const Colorf &color) {
  _color = color;
  _flags |= F_has_color;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAttributes::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggAttributes::
clear_color() {
  _flags &= ~F_has_color;
}
