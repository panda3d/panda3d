// Filename: LinearSourceForce.cxx
// Created by:  charles (21Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "linearSourceForce.h"

TypeHandle LinearSourceForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : LinearSourceForce
//      Access : Public
// Description : Simple constructor
////////////////////////////////////////////////////////////////////
LinearSourceForce::
LinearSourceForce(const LPoint3f& p, FalloffType f, float r, float a, 
          bool mass) :
  LinearDistanceForce(p, f, r, a, mass) {
}

////////////////////////////////////////////////////////////////////
//    Function : LinearSourceForce
//      Access : Public
// Description : Simple constructor
////////////////////////////////////////////////////////////////////
LinearSourceForce::
LinearSourceForce(void) :
  LinearDistanceForce(LPoint3f(0.0f, 0.0f, 0.0f), FT_ONE_OVER_R_SQUARED, 
                      1.0f, 1.0f, true) {
}

////////////////////////////////////////////////////////////////////
//    Function : LinearSourceForce
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
LinearSourceForce::
LinearSourceForce(const LinearSourceForce &copy) :
  LinearDistanceForce(copy) {
}

////////////////////////////////////////////////////////////////////
//    Function : ~LinearSourceForce
//      Access : Public
// Description : Simple destructor
////////////////////////////////////////////////////////////////////
LinearSourceForce::
~LinearSourceForce(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
LinearForce *LinearSourceForce::
make_copy(void) {
  return new LinearSourceForce(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : get_child_vector
//      Access : Public
// Description : virtual force query
////////////////////////////////////////////////////////////////////
LVector3f LinearSourceForce::
get_child_vector(const PhysicsObject *po) {
  return (po->get_position() - get_force_center()) * get_scalar_term();
}
