/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file angularVectorForce.cxx
 * @author charles
 * @date 2000-08-09
 */

#include "angularVectorForce.h"

TypeHandle AngularVectorForce::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AngularVectorForce
//       Access: Public
//  Description: constructor
////////////////////////////////////////////////////////////////////
AngularVectorForce::
AngularVectorForce(const LRotation &vec) :
  AngularForce(), _fvec(vec) {
}

////////////////////////////////////////////////////////////////////
//     Function: AngularVectorForce
//       Access: Public
//  Description: constructor
////////////////////////////////////////////////////////////////////
AngularVectorForce::
AngularVectorForce(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) :
  AngularForce() {
  _fvec.set_hpr(LVecBase3(h, p, r));
}

////////////////////////////////////////////////////////////////////
//     Function: AngularVectorForce
//       Access: Public
//  Description: copy constructor
////////////////////////////////////////////////////////////////////
AngularVectorForce::
AngularVectorForce(const AngularVectorForce &copy) :
  AngularForce(copy) {
  _fvec = copy._fvec;
}

////////////////////////////////////////////////////////////////////
//     Function: ~AngularVectorForce
//       Access: Public, Virtual
//  Description: destructor
////////////////////////////////////////////////////////////////////
AngularVectorForce::
~AngularVectorForce() {
}

////////////////////////////////////////////////////////////////////
//     Function: make_copy
//       Access: Private, Virtual
//  Description: dynamic copier
////////////////////////////////////////////////////////////////////
AngularForce *AngularVectorForce::
make_copy() const {
  return new AngularVectorForce(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: get_child_quat
//       Access: Private, Virtual
//  Description: query
////////////////////////////////////////////////////////////////////
LRotation AngularVectorForce::
get_child_quat(const PhysicsObject *) {
  return _fvec;
}

////////////////////////////////////////////////////////////////////
//     Function: output
//       Access: Public
//  Description: Write a string representation of this instance to
//               <out>.
////////////////////////////////////////////////////////////////////
void AngularVectorForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"AngularVectorForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: write
//       Access: Public
//  Description: Write a string representation of this instance to
//               <out>.
////////////////////////////////////////////////////////////////////
void AngularVectorForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"AngularVectorForce:\n";
  out.width(indent+2); out<<""; out<<"_fvec "<<_fvec<<"\n";
  AngularForce::write(out, indent+2);
  #endif //] NDEBUG
}
