// Filename: angularVectorForce.cxx
// Created by:  charles (09Aug00)
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

#include "angularVectorForce.h"

TypeHandle AngularVectorForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : AngularVectorForce
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////
AngularVectorForce::
AngularVectorForce(const LVector3f &vec) :
  AngularForce(), _fvec(vec) {
}

////////////////////////////////////////////////////////////////////
//    Function : AngularVectorForce
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////
AngularVectorForce::
AngularVectorForce(float x, float y, float z) :
  AngularForce() {
  _fvec.set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//    Function : AngularVectorForce
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
AngularVectorForce::
AngularVectorForce(const AngularVectorForce &copy) :
  AngularForce(copy) {
  _fvec = copy._fvec;
}

////////////////////////////////////////////////////////////////////
//    Function : ~AngularVectorForce
//      Access : public, virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
AngularVectorForce::
~AngularVectorForce(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : private, virtual
// Description : dynamic copier
////////////////////////////////////////////////////////////////////
AngularForce *AngularVectorForce::
make_copy(void) const {
  return new AngularVectorForce(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : get_child_vector
//      Access : private, virtual
// Description : query
////////////////////////////////////////////////////////////////////
LVector3f AngularVectorForce::
get_child_vector(const PhysicsObject *) {
  return _fvec;
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void AngularVectorForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"AngularVectorForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void AngularVectorForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"AngularVectorForce:\n";
  out.width(indent+2); out<<""; out<<"_fvec "<<_fvec<<"\n";
  AngularForce::write(out, indent+2);
  #endif //] NDEBUG
}
