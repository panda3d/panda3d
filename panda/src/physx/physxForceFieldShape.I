/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxForceFieldShape.I
 * @author enn0x
 * @date 2009-11-15
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxForceFieldShape::
PhysxForceFieldShape() : PhysxObject() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxForceFieldShape::
ls() const {

  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxForceFieldShape::
ls(ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " " << _name
                            << " (at 0x" << this << ")\n";
}

