// Filename: baseParticle.cxx
// Created by:  charles (14Jun00)
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

#include "baseParticle.h"

////////////////////////////////////////////////////////////////////
//    Function : BaseParticle
//      Access : Public
// Description : Default Constructor
////////////////////////////////////////////////////////////////////
BaseParticle::
BaseParticle(int lifespan, bool alive) :
  _age(0.0f), _lifespan(lifespan), _alive(alive) {
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticle
//      Access : Public
// Description : Copy Constructor
////////////////////////////////////////////////////////////////////
BaseParticle::
BaseParticle(const BaseParticle &copy) {
  _age = copy._age;
  _lifespan = copy._lifespan;
  _alive = copy._alive;
}

////////////////////////////////////////////////////////////////////
//    Function : ~BaseParticle
//      Access : Public
// Description : Default Destructor
////////////////////////////////////////////////////////////////////
BaseParticle::
~BaseParticle(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : get_theta
//      Access : Public
// Description : for spriteParticleRenderer
////////////////////////////////////////////////////////////////////
float BaseParticle::
get_theta(void) const {
  return 0.0f;
}
