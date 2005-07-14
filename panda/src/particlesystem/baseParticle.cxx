// Filename: baseParticle.cxx
// Created by:  charles (14Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "baseParticle.h"

////////////////////////////////////////////////////////////////////
//    Function : BaseParticle
//      Access : Public
// Description : Default Constructor
////////////////////////////////////////////////////////////////////
BaseParticle::
BaseParticle(float lifespan, bool alive) :
  _age(0.0f), _lifespan(lifespan), _alive(alive), _index(0){
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticle
//      Access : Public
// Description : Copy Constructor
////////////////////////////////////////////////////////////////////
BaseParticle::
BaseParticle(const BaseParticle &copy) :
  _age(copy._age),
  _lifespan(copy._lifespan),
  _alive(copy._alive),
  _index(copy._index) {
}

////////////////////////////////////////////////////////////////////
//    Function : ~BaseParticle
//      Access : Public
// Description : Default Destructor
////////////////////////////////////////////////////////////////////
BaseParticle::
~BaseParticle() {
}

////////////////////////////////////////////////////////////////////
//    Function : get_theta
//      Access : Public
// Description : for spriteParticleRenderer
////////////////////////////////////////////////////////////////////
float BaseParticle::
get_theta() const {
  return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseParticle::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"BaseParticle";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseParticle::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"BaseParticle:\n";
  out.width(indent+2); out<<""; out<<"_age "<<_age<<"\n";
  out.width(indent+2); out<<""; out<<"_lifespan "<<_lifespan<<"\n";
  out.width(indent+2); out<<""; out<<"_alive "<<_alive<<"\n";
  out.width(indent+2); out<<""; out<<"_index "<<_index<<"\n";
  out.width(indent+2); out<<""; out<<"_last_position "<<_last_position<<"\n";
  PhysicsObject::write(out, indent+2);
  #endif //] NDEBUG
}

