// Filename: physxActor.I
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
//     Function: PhysxActor::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxActor::
PhysxActor() : PhysxObject() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxActor::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxActor::
~PhysxActor() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxActor::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxActor::
ls() const {

  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxActor::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxActor::
ls(ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " " << _name
                            << " (at 0x" << this << ")";

  if (!_np.is_empty()) {
    out << " NP:" << _np;
  }

  if (_controller) {
    out << " C:0x" << _controller;
  }

  out << "\n";

  _shapes.ls(out, indent_level);
}

