/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxClothMesh.I
 * @author enn0x
 * @date 2010-03-28
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothMesh::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxClothMesh::
PhysxClothMesh() : PhysxObject() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothMesh::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxClothMesh::
~PhysxClothMesh() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothMesh::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxClothMesh::
ls() const {

  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothMesh::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxClothMesh::
ls(ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " (at 0x" << this << ")\n";
}

