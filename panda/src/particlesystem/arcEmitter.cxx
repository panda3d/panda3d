// Filename: arcEmitter.cxx
// Created by:  charles (22Jun00)
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

#include "arcEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : ArcEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
ArcEmitter::
ArcEmitter() :
  RingEmitter(), _start_theta(0.0f), _end_theta(MathNumbers::pi_f)
{
}

////////////////////////////////////////////////////////////////////
//    Function : ArcEmitter
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
ArcEmitter::
ArcEmitter(const ArcEmitter &copy) :
  RingEmitter(copy) {
  _start_theta = copy._start_theta;
  _end_theta = copy._end_theta;
}

////////////////////////////////////////////////////////////////////
//    Function : ~ArcEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
ArcEmitter::
~ArcEmitter() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *ArcEmitter::
make_copy() {
  return new ArcEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : ArcEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void ArcEmitter::
assign_initial_position(LPoint3& pos) {
  PN_stdfloat theta;
  if ( _start_theta < _end_theta ) {
    theta = LERP(NORMALIZED_RAND(), _start_theta, _end_theta);
  } else {
    theta = LERP(NORMALIZED_RAND(), _start_theta, _end_theta + 2.0f * MathNumbers::pi_f);
  }    
  
  theta += (MathNumbers::pi_f / 2.0);
  this->_cos_theta = cosf(theta);
  this->_sin_theta = sinf(theta);

  PN_stdfloat new_radius_spread = SPREAD(_radius_spread);
  PN_stdfloat new_x = _cos_theta * (_radius + new_radius_spread);
  PN_stdfloat new_y = _sin_theta * (_radius + new_radius_spread);

  pos.set(new_x, new_y, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a starc representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void ArcEmitter::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"ArcEmitter";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a starc representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void ArcEmitter::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"ArcEmitter:\n";
  out.width(indent+2); out<<""; out<<"_start_angle "<<rad_2_deg(_start_theta)<<"\n";
  out.width(indent+2); out<<""; out<<"_end_angle "<<rad_2_deg(_end_theta)<<"\n";
  RingEmitter::write(out, indent+2);
  #endif //] NDEBUG
}
