// Filename: LinearDistanceForce.cxx
// Created by:  charles (21Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "linearDistanceForce.h"

TypeHandle LinearDistanceForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : LinearDistanceForce
//      Access : Protected
// Description : Simple constructor
////////////////////////////////////////////////////////////////////
LinearDistanceForce::
LinearDistanceForce(const LPoint3f& p, FalloffType ft, float r, float a, bool m) :
  _falloff(ft), _radius(r), _force_center(p),
  LinearForce(a, m) {
}

////////////////////////////////////////////////////////////////////
//    Function : LinearDistanceForce
//      Access : Protected
// Description : copy constructor
////////////////////////////////////////////////////////////////////
LinearDistanceForce::
LinearDistanceForce(const LinearDistanceForce &copy) :
  LinearForce(copy) {
  _falloff = copy._falloff;
  _radius = copy._radius;
  _force_center = copy._force_center;
}

////////////////////////////////////////////////////////////////////
//    Function : ~LinearDistanceForce
//      Access : Protected
// Description : destructor
////////////////////////////////////////////////////////////////////
LinearDistanceForce::
~LinearDistanceForce(void) {
}
