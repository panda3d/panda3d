// Filename: LinearVectorForce.cxx
// Created by:  charles (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

#include "linearVectorForce.h"

TypeHandle LinearVectorForce::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : LinearVectorForce 
//       Access : Public
//  Description : Vector Constructor
////////////////////////////////////////////////////////////////////
LinearVectorForce::
LinearVectorForce(const LVector3f& vec, float a, bool mass) : 
  LinearForce(a, mass),
  _fvec(vec) 
{
}

////////////////////////////////////////////////////////////////////
//     Function : LinearVectorForce
//       Access : Public
//  Description : Default/Piecewise constructor
////////////////////////////////////////////////////////////////////
LinearVectorForce::
LinearVectorForce(float x, float y, float z, float a, bool mass) :
  LinearForce(a, mass) {
  _fvec.set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function : LinearVectorForce
//       Access : Public
//  Description : Copy Constructor
////////////////////////////////////////////////////////////////////
LinearVectorForce::
LinearVectorForce(const LinearVectorForce &copy) :
  LinearForce(copy) {
  _fvec = copy._fvec;
}

////////////////////////////////////////////////////////////////////
//     Function : LinearVectorForce
//       Access : Public
//  Description : Destructor
////////////////////////////////////////////////////////////////////
LinearVectorForce::
~LinearVectorForce(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public, virtual
// Description : copier
////////////////////////////////////////////////////////////////////
LinearForce *LinearVectorForce::
make_copy(void) {
  return new LinearVectorForce(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : get_child_vector
//      Access : Public
// Description : vector access
////////////////////////////////////////////////////////////////////
LVector3f LinearVectorForce::
get_child_vector(const PhysicsObject *) {
  return _fvec;
}

