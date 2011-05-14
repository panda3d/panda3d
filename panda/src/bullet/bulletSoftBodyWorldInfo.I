// Filename: bulletSoftBodyWorldInfo.I
// Created by:  enn0x (04Mar10)
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
//     Function: BulletSoftBodyWorldInfo::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletSoftBodyWorldInfo::
~BulletSoftBodyWorldInfo() {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_info
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE btSoftBodyWorldInfo &BulletSoftBodyWorldInfo::
get_info() const {

  return _info;
}

