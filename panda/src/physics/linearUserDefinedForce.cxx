// Filename: linearUserDefinedForce.cxx
// Created by:  charles (31Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "linearUserDefinedForce.h"

TypeHandle LinearUserDefinedForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : LinearUserDefinedForce
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////
LinearUserDefinedForce::
LinearUserDefinedForce(LVector3f (*proc)(const PhysicsObject *),
                 float a, bool md) :
  LinearForce(a, md),
  _proc(proc)
{
}

////////////////////////////////////////////////////////////////////
//    Function : LinearUserDefinedForce
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
LinearUserDefinedForce::
LinearUserDefinedForce(const LinearUserDefinedForce &copy) :
  LinearForce(copy) {
  _proc = copy._proc;
}

////////////////////////////////////////////////////////////////////
//    Function : ~LinearUserDefinedForce
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////
LinearUserDefinedForce::
~LinearUserDefinedForce(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : private, virtual
// Description : child copier
////////////////////////////////////////////////////////////////////
LinearForce *LinearUserDefinedForce::
make_copy(void) {
  return new LinearUserDefinedForce(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : get_child_vector
//      Access : private, virtual
// Description : force builder
////////////////////////////////////////////////////////////////////
LVector3f LinearUserDefinedForce::
get_child_vector(const PhysicsObject *po) {
  return _proc(po);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearUserDefinedForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearUserDefinedForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearUserDefinedForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearUserDefinedForce:\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}
