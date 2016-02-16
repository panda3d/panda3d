/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearUserDefinedForce.cxx
 * @author charles
 * @date 2000-07-31
 */

#include "linearUserDefinedForce.h"

TypeHandle LinearUserDefinedForce::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LinearUserDefinedForce
//       Access: Public
//  Description: constructor
////////////////////////////////////////////////////////////////////
LinearUserDefinedForce::
LinearUserDefinedForce(LVector3 (*proc)(const PhysicsObject *),
    PN_stdfloat a, bool md) :
  LinearForce(a, md),
  _proc(proc)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LinearUserDefinedForce
//       Access: Public
//  Description: copy constructor
////////////////////////////////////////////////////////////////////
LinearUserDefinedForce::
LinearUserDefinedForce(const LinearUserDefinedForce &copy) :
  LinearForce(copy) {
  _proc = copy._proc;
}

////////////////////////////////////////////////////////////////////
//     Function: ~LinearUserDefinedForce
//       Access: Public
//  Description: destructor
////////////////////////////////////////////////////////////////////
LinearUserDefinedForce::
~LinearUserDefinedForce() {
}

////////////////////////////////////////////////////////////////////
//     Function: make_copy
//       Access: Private, Virtual
//  Description: child copier
////////////////////////////////////////////////////////////////////
LinearForce *LinearUserDefinedForce::
make_copy() {
  return new LinearUserDefinedForce(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: get_child_vector
//       Access: Private, Virtual
//  Description: force builder
////////////////////////////////////////////////////////////////////
LVector3 LinearUserDefinedForce::
get_child_vector(const PhysicsObject *po) {
  return _proc(po);
}

////////////////////////////////////////////////////////////////////
//     Function: output
//       Access: Public
//  Description: Write a string representation of this instance to
//               <out>.
////////////////////////////////////////////////////////////////////
void LinearUserDefinedForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearUserDefinedForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: write
//       Access: Public
//  Description: Write a string representation of this instance to
//               <out>.
////////////////////////////////////////////////////////////////////
void LinearUserDefinedForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearUserDefinedForce:\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}
