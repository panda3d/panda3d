// Filename: linearControlForce.cxx
// Created by: Dave Schuyler (2006)
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

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

#include "linearControlForce.h"

TypeHandle LinearControlForce::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : LinearControlForce
//       Access : Public
//  Description : Vector Constructor
////////////////////////////////////////////////////////////////////
LinearControlForce::
LinearControlForce(const PhysicsObject *po, PN_stdfloat a, bool mass) :
  LinearForce(a, mass),
  _physics_object(po),
  _fvec(0.0f, 0.0f, 0.0f) {
}

////////////////////////////////////////////////////////////////////
//     Function : LinearControlForce
//       Access : Public
//  Description : Copy Constructor
////////////////////////////////////////////////////////////////////
LinearControlForce::
LinearControlForce(const LinearControlForce &copy) :
  LinearForce(copy) {
  _physics_object = copy._physics_object;
  _fvec = copy._fvec;
}

////////////////////////////////////////////////////////////////////
//     Function : LinearControlForce
//       Access : Public
//  Description : Destructor
////////////////////////////////////////////////////////////////////
LinearControlForce::
~LinearControlForce() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public, virtual
// Description : copier
////////////////////////////////////////////////////////////////////
LinearForce *LinearControlForce::
make_copy() {
  return new LinearControlForce(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : get_child_vector
//      Access : Public
// Description : vector access
////////////////////////////////////////////////////////////////////
LVector3 LinearControlForce::
get_child_vector(const PhysicsObject *po) {
  if (_physics_object != 0 && po == _physics_object) {
    return _fvec;
  } else {
    return LVector3::zero();
  }
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearControlForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearControlForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearControlForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearControlForce:\n";
  out.width(indent+2); out<<""; out<<"_fvec "<<_fvec<<"\n";
  out.width(indent+2); out<<""; out<<"_physics_object "<<_physics_object<<"\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}
