// Filename: LinearUserDefinedForce.cxx
// Created by:  charles (31Jul00)
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
