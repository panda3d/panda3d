/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCcdSkeleton.I
 * @author enn0x
 * @date 2012-05-01
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxCcdSkeleton::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxCcdSkeleton::
PhysxCcdSkeleton() : PhysxObject() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCcdSkeleton::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxCcdSkeleton::
~PhysxCcdSkeleton() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCcdSkeleton::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxCcdSkeleton::
ls() const {

  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCcdSkeleton::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxCcdSkeleton::
ls(ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " (at 0x" << this << ")\n";
}

