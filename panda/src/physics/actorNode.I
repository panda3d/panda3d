// Filename: actorNode.I
// Created by:  charles (07Aug00)
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
//    Function : set_contact_vector
//      Access : Public
// Description : 
////////////////////////////////////////////////////////////////////
INLINE void ActorNode::
set_contact_vector(const LVector3 &contact_vector) {
  _contact_vector=contact_vector;
}

////////////////////////////////////////////////////////////////////
//    Function : get_contact_vector
//      Access : Public
// Description : 
////////////////////////////////////////////////////////////////////
INLINE const LVector3 &ActorNode::
get_contact_vector() const {
  return _contact_vector;
}
