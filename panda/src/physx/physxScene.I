// Filename: physxScene.I
// Created by:  enn0x (14Sep09)
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
//     Function: PhysxScene::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxScene::
PhysxScene() : PhysxObject() {

  _debugNode = new PhysxDebugGeomNode();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxScene::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxScene::
~PhysxScene() {

  _debugNode = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxScene::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxScene::
ls() const {

  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxScene::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxScene::
ls(ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " (at 0x" << this << ")\n";

  _materials.ls(out, indent_level);
  _actors.ls(out, indent_level);
  _joints.ls(out, indent_level);
  _forcefields.ls(out, indent_level);
  _ffgroups.ls(out, indent_level);
  _controllers.ls(out, indent_level);
  _vehicles.ls(out, indent_level);
  _cloths.ls(out, indent_level);
  _softbodies.ls(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxScene::set_controller_shape_hit_callback
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxScene::
set_controller_shape_hit_callback(PT(CallbackObject) cbobj) {

  _controller_report.set_shape_hit_callback(cbobj);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxScene::set_controller_controller_hit_callback
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxScene::
set_controller_controller_hit_callback(PT(CallbackObject) cbobj) {

  _controller_report.set_controller_hit_callback(cbobj);
}

