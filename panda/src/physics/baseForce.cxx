// Filename: baseForce.cxx
// Created by:  charles (08Aug00)
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

#include "baseForce.h"

TypeHandle BaseForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : BaseForce
//      Access : protected
// Description : constructor
////////////////////////////////////////////////////////////////////
BaseForce::
BaseForce(bool active) :
  _force_node(NULL), _active(active) {
}

////////////////////////////////////////////////////////////////////
//    Function : BaseForce
//      Access : protected
// Description : copy constructor
////////////////////////////////////////////////////////////////////
BaseForce::
BaseForce(const BaseForce &copy) :
  TypedReferenceCount(copy) {
  _active = copy._active;
  _force_node = (ForceNode *) NULL;
}

////////////////////////////////////////////////////////////////////
//    Function : ~BaseForce
//      Access : public, virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
BaseForce::
~BaseForce() {
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"BaseForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"BaseForce:\n";
  out.width(indent+2); out<<""; out<<"_force_node "<<_force_node<<"\n";
  out.width(indent+2); out<<""; out<<"_active "<<_active<<"\n";
  //TypedReferenceCount::write(out);
  #endif //] NDEBUG
}
