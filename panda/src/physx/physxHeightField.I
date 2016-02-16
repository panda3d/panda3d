/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxHeightField.I
 * @author enn0x
 * @date 2009-10-15
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightField::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxHeightField::
PhysxHeightField() : PhysxObject() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightField::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxHeightField::
~PhysxHeightField() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightField::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxHeightField::
ls() const {

  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightField::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxHeightField::
ls(ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " (at 0x" << this << ")\n";
}

