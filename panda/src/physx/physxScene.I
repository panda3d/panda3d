/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxScene.I
 * @author enn0x
 * @date 2009-09-14
 */

/**
 *
 */
INLINE PhysxScene::
PhysxScene() : PhysxObject() {

  _debugNode = new PhysxDebugGeomNode();
}

/**
 *
 */
INLINE PhysxScene::
~PhysxScene() {

  _debugNode = nullptr;
}

/**
 *
 */
INLINE void PhysxScene::
ls() const {

  ls(nout);
}

/**
 *
 */
INLINE void PhysxScene::
ls(std::ostream &out, int indent_level) const {

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

/**
 *
 */
INLINE void PhysxScene::
set_controller_shape_hit_callback(PT(CallbackObject) cbobj) {

  _controller_report.set_shape_hit_callback(cbobj);
}

/**
 *
 */
INLINE void PhysxScene::
set_controller_controller_hit_callback(PT(CallbackObject) cbobj) {

  _controller_report.set_controller_hit_callback(cbobj);
}
