/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMaterial.I
 * @author enn0x
 * @date 2009-09-21
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxMaterial::
PhysxMaterial() : PhysxObject() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxMaterial::
~PhysxMaterial() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxMaterial::
ls() const {

  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxMaterial::
ls(ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " (at 0x" << this << ")\n";
}

