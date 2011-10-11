// Filename: zSpinParticle.cxx
// Created by:  charles (16Aug00)
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

#include "zSpinParticle.h"
#include "cmath.h"

////////////////////////////////////////////////////////////////////
//    Function : ZSpinParticle
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////
ZSpinParticle::
ZSpinParticle() :
  BaseParticle() {
  _initial_angle = 0.0f;
  _final_angle = 0.0f;
  _cur_angle = 0.0f;
  _angular_velocity = 0.0f;
  _bUseAngularVelocity = false;
}

////////////////////////////////////////////////////////////////////
//    Function : ZSpinParticle
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
ZSpinParticle::
ZSpinParticle(const ZSpinParticle &copy) :
  BaseParticle(copy) {
  _initial_angle = copy._initial_angle;
  _final_angle = copy._final_angle;
  _cur_angle = copy._cur_angle;
  _angular_velocity = copy._angular_velocity;
  _bUseAngularVelocity = copy._bUseAngularVelocity;
}

////////////////////////////////////////////////////////////////////
//    Function : ~ZSpinParticle
//      Access : public, virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
ZSpinParticle::
~ZSpinParticle() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : public, virtual
// Description : dynamic copier
////////////////////////////////////////////////////////////////////
PhysicsObject *ZSpinParticle::
make_copy() const {
  return new ZSpinParticle(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : init
//      Access : public, virtual
// Description :
////////////////////////////////////////////////////////////////////
void ZSpinParticle::
init() {
}

////////////////////////////////////////////////////////////////////
//    Function : update
//      Access : public, virtual
// Description :
////////////////////////////////////////////////////////////////////
void ZSpinParticle::
update() {
  // if using final_angle, want age to range from [0,1] over lifespan, so use parameterized_age
  // for angular velocity, should be allowed to range freely upward, use regular age

  if(_bUseAngularVelocity) {
   // interpolate the current orientation
      _cur_angle = _initial_angle + (get_age() * _angular_velocity);
  } else {
      _cur_angle = _initial_angle + (get_parameterized_age() * (_final_angle - _initial_angle));
  }

  // normalize the result to [0..360)
  _cur_angle = cmod(_cur_angle, (PN_stdfloat)360.0);

  // if _cur_angle was negative, it is still negative after cmod,
  // wrap it around by adding 360

  // is this really necessary?  should be in range of sin/cos
  if(_cur_angle < 0.0f)
    _cur_angle += 360.0f;
}

////////////////////////////////////////////////////////////////////
//    Function : die
//      Access : public, virtual
// Description :
////////////////////////////////////////////////////////////////////
void ZSpinParticle::
die() {
}

////////////////////////////////////////////////////////////////////
//    Function : get_theta
//      Access : public, virtual
// Description :
////////////////////////////////////////////////////////////////////
PN_stdfloat ZSpinParticle::
get_theta() const {
  return _cur_angle;
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void ZSpinParticle::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"ZSpinParticle";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void ZSpinParticle::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"ZSpinParticle:\n";
  out.width(indent+2); out<<""; out<<"_initial_angle "<<_initial_angle<<"\n";
  out.width(indent+2); out<<""; out<<"_final_angle "<<_final_angle<<"\n";
  out.width(indent+2); out<<""; out<<"_cur_angle "<<_cur_angle<<"\n";
  out.width(indent+2); out<<""; out<<"_angular_velocity "<<_angular_velocity<<"\n";
  out.width(indent+2); out<<""; out<<"_bUseAngularVelocity "<<_bUseAngularVelocity<<"\n";
  BaseParticle::write(out, indent+2);
  #endif //] NDEBUG
}
