// Filename: angularForce.cxx
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

#include "angularForce.h"

TypeHandle AngularForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : AngularForce
//      Access : protected
// Description : constructor
////////////////////////////////////////////////////////////////////
AngularForce::
AngularForce() :
  BaseForce() {
}

////////////////////////////////////////////////////////////////////
//    Function : AngularForce
//      Access : protected
// Description : copy constructor
////////////////////////////////////////////////////////////////////
AngularForce::
AngularForce(const AngularForce &copy) :
  BaseForce(copy) {
}

////////////////////////////////////////////////////////////////////
//    Function : ~AngularForce
//      Access : public, virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
AngularForce::
~AngularForce() {
}

////////////////////////////////////////////////////////////////////
//    Function : get_quat
//      Access : public
// Description : access query
////////////////////////////////////////////////////////////////////
LRotationf AngularForce::
get_quat(const PhysicsObject *po) {
  LRotationf v = get_child_quat(po);
  return v;
}

////////////////////////////////////////////////////////////////////
//    Function : is_linear
//      Access : public
// Description : access query
////////////////////////////////////////////////////////////////////
bool AngularForce::
is_linear() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void AngularForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"AngularForce (id "<<this<<")";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void AngularForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"AngularForce (id "<<this<<")\n";
  BaseForce::write(out, indent+2);
  #endif //] NDEBUG
}
