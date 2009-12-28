// Filename: physxKitchen.I
// Created by:  enn0x (12Oct09)
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
//     Function: PhysxKitchen::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxKitchen::
PhysxKitchen() {

  _cooking = NxGetCookingLib( NX_PHYSICS_SDK_VERSION );

  bool status = _cooking->NxInitCooking();
  nassertv_always( status );
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxKitchen::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxKitchen::
~PhysxKitchen() {

  _cooking->NxCloseCooking();
}

