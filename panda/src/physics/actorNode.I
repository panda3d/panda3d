// Filename: actorNode.I
// Created by:  charles (07Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//    Function : set_contact_vector
//      Access : Public
// Description : 
////////////////////////////////////////////////////////////////////
INLINE void ActorNode::
set_contact_vector(const LVector3f &contact_vector) {
  _contact_vector=contact_vector;
}

////////////////////////////////////////////////////////////////////
//    Function : get_contact_vector
//      Access : Public
// Description : 
////////////////////////////////////////////////////////////////////
INLINE const LVector3f &ActorNode::
get_contact_vector() const {
  return _contact_vector;
}
