// Filename: physxControllerReport.I
// Created by:  enn0x (24Sep09)
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
//     Function: PhysxControllerReport::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxControllerReport::
PhysxControllerReport() : NxUserControllerHitReport() {

  _enabled = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerReport::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxControllerReport::
~PhysxControllerReport() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerReport::set_shape_callback
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PhysxControllerReport::
set_shape_hit_callback(PT(CallbackObject) cbobj) {

  _shape_hit_cbobj = cbobj;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerReport::set_shape_callback
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PhysxControllerReport::
set_controller_hit_callback(PT(CallbackObject) cbobj) {

  _controller_hit_cbobj = cbobj;
}

