// Filename: physxForceFieldShapeGroup.I
// Created by:  enn0x (11Nov09)
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
//     Function: PhysxForceFieldShapeGroup::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxForceFieldShapeGroup::
PhysxForceFieldShapeGroup() : PhysxObject() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxForceFieldShapeGroup::
~PhysxForceFieldShapeGroup() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxForceFieldShapeGroup::
ls() const {

  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxForceFieldShapeGroup::
ls(ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " " << _name
                            << " (at 0x" << this << ")\n";

  _shapes.ls(out, indent_level);
}

