/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file angularForce.cxx
 * @author charles
 * @date 2000-08-08
 */

#include "angularForce.h"

TypeHandle AngularForce::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AngularForce
//       Access: Protected
//  Description: constructor
////////////////////////////////////////////////////////////////////
AngularForce::
AngularForce() :
  BaseForce() {
}

////////////////////////////////////////////////////////////////////
//     Function: AngularForce
//       Access: Protected
//  Description: copy constructor
////////////////////////////////////////////////////////////////////
AngularForce::
AngularForce(const AngularForce &copy) :
  BaseForce(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: ~AngularForce
//       Access: Public, Virtual
//  Description: destructor
////////////////////////////////////////////////////////////////////
AngularForce::
~AngularForce() {
}

////////////////////////////////////////////////////////////////////
//     Function: get_quat
//       Access: Public
//  Description: access query
////////////////////////////////////////////////////////////////////
LRotation AngularForce::
get_quat(const PhysicsObject *po) {
  LRotation v = get_child_quat(po);
  return v;
}

////////////////////////////////////////////////////////////////////
//     Function: is_linear
//       Access: Public
//  Description: access query
////////////////////////////////////////////////////////////////////
bool AngularForce::
is_linear() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: output
//       Access: Public
//  Description: Write a string representation of this instance to
//               <out>.
////////////////////////////////////////////////////////////////////
void AngularForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"AngularForce (id "<<this<<")";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: write
//       Access: Public
//  Description: Write a string representation of this instance to
//               <out>.
////////////////////////////////////////////////////////////////////
void AngularForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"AngularForce (id "<<this<<")\n";
  BaseForce::write(out, indent+2);
  #endif //] NDEBUG
}
