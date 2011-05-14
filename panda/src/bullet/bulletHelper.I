// Filename: bulletHelper.I
// Created by:  enn0x (19Jan11)
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
//     Function: BulletHelper::get_sb_index
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) BulletHelper::
get_sb_index() {

  if (_sb_index == (InternalName *)NULL) {
    _sb_index = InternalName::make("sb_index");
  }
  return _sb_index;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHelper::get_sb_flip
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PT(InternalName) BulletHelper::
get_sb_flip() {

  if (_sb_flip == (InternalName *)NULL) {
    _sb_flip = InternalName::make("sb_flip");
  }
  return _sb_flip;
}
