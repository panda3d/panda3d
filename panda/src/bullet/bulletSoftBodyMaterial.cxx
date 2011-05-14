// Filename: bulletSoftBodyMaterial.cxx
// Created by:  enn0x (19Mar11)
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

#include "bulletSoftBodyMaterial.h"

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyMaterial::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyMaterial::
BulletSoftBodyMaterial(btSoftBody::Material &material) : _material(material) {

}

