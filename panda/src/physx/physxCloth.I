// Filename: physxCloth.I
// Created by:  enn0x (30Mar10)
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
//     Function: PhysxCloth::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxCloth::
PhysxCloth() : PhysxObject() {

  _name = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCloth::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxCloth::
~PhysxCloth() {

  free(_name);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCloth::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxCloth::
ls() const {

  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCloth::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxCloth::
ls(ostream &out, int indent_level) const {

  const char *name = _name ? _name : "";

  indent(out, indent_level) << get_type().get_name()
                            << " " << name
                            << " (at 0x" << this << ")";

  //if (_node) {
  //  " N:0x" << _node;
  //}

  out << "\n";
}

