/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file orientedParticle.cxx
 * @author charles
 * @date 2000-06-19
 */

#include "orientedParticle.h"

////////////////////////////////////////////////////////////////////
//     Function: OrientedParticle
//       Access: Public
//  Description: simple constructor
////////////////////////////////////////////////////////////////////
OrientedParticle::
OrientedParticle(int lifespan, bool alive) :
  BaseParticle(lifespan, alive) {
  set_oriented(true);
}

////////////////////////////////////////////////////////////////////
//     Function: OrientedParticle
//       Access: Public
//  Description: copy constructor
////////////////////////////////////////////////////////////////////
OrientedParticle::
OrientedParticle(const OrientedParticle &copy) :
  BaseParticle(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: ~OrientedParticle
//       Access: Public
//  Description: simple destructor
////////////////////////////////////////////////////////////////////
OrientedParticle::
~OrientedParticle() {
}

////////////////////////////////////////////////////////////////////
//     Function: make_copy
//       Access: Public, Virtual
//  Description: simple destructor
////////////////////////////////////////////////////////////////////
PhysicsObject *OrientedParticle::
make_copy() const {
  return new OrientedParticle(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: init
//       Access: Public
//  Description: particle init routine
////////////////////////////////////////////////////////////////////
void OrientedParticle::
init() {
}

////////////////////////////////////////////////////////////////////
//     Function: die
//       Access: Public
//  Description: particle death routine
////////////////////////////////////////////////////////////////////
void OrientedParticle::
die() {
}

////////////////////////////////////////////////////////////////////
//     Function: update
//       Access: Public
//  Description: particle update routine.
//               This NEEDS to be filled in with quaternion slerp
//               stuff, or oriented particles will not rotate.
////////////////////////////////////////////////////////////////////
void OrientedParticle::
update() {
}

////////////////////////////////////////////////////////////////////
//     Function: output
//       Access: Public
//  Description: Write a string representation of this instance to
//               <out>.
////////////////////////////////////////////////////////////////////
void OrientedParticle::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"OrientedParticle";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: write
//       Access: Public
//  Description: Write a string representation of this instance to
//               <out>.
////////////////////////////////////////////////////////////////////
void OrientedParticle::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"OrientedParticle:\n";
  BaseParticle::write(out, indent+2);
  #endif //] NDEBUG
}
