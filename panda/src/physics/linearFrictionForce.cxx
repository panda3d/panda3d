// Filename: LinearFrictionForce.cxx
// Created by:  charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "linearFrictionForce.h"

TypeHandle LinearFrictionForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : LinearFrictionForce
//      Access : Public
// Description : Constructor
////////////////////////////////////////////////////////////////////
LinearFrictionForce::
LinearFrictionForce(float coef, float a, bool m) :
  LinearForce(a, m) {

  // friction REALLY shouldn't be outside of [0, 1]
  if (coef < 0.0f)
    coef = 0.0f;
  else if (coef > 1.0f)
    coef = 1.0f;

  _coef = coef;
}

////////////////////////////////////////////////////////////////////
//    Function : LinearFrictionForce
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
LinearFrictionForce::
LinearFrictionForce(const LinearFrictionForce &copy) :
  LinearForce(copy) {
  _coef = copy._coef;
}

////////////////////////////////////////////////////////////////////
//    Function : LinearFrictionForce
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
LinearFrictionForce::
~LinearFrictionForce(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
LinearForce *LinearFrictionForce::
make_copy(void) {
  return new LinearFrictionForce(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : LinearFrictionForce
//      Access : Public
// Description : Constructor
////////////////////////////////////////////////////////////////////
LVector3f LinearFrictionForce::
get_child_vector(const PhysicsObject* po) {
  LVector3f v = po->get_velocity();
  LVector3f friction = -v * (1.0f - _coef);

  // cary said to cap this at zero so that friction can't reverse
  // your direction, but it seems to me that if you're computing:
  //     v + (-v * _coef), _coef in [0, 1]
  // that this will always be greater than or equal to zero.
  return friction;
}
