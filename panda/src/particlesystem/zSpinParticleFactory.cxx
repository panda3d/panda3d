// Filename: zSpinParticleFactory.cxx
// Created by:  charles (16Aug00)
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

#include "zSpinParticleFactory.h"
#include "zSpinParticle.h"

////////////////////////////////////////////////////////////////////
//    Function : ZSpinParticleFactory
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////
ZSpinParticleFactory::
ZSpinParticleFactory() :
  BaseParticleFactory() {
  _initial_angle = 0.0f;
  _final_angle = 0.0f;
  _initial_angle_spread = 0.0f;
  _final_angle_spread = 0.0f;
  _angular_velocity = 0.0f;
  _angular_velocity_spread = 0.0f;
  _bUseAngularVelocity = false;
}

////////////////////////////////////////////////////////////////////
//    Function : ZSpinParticleFactory
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
ZSpinParticleFactory::
ZSpinParticleFactory(const ZSpinParticleFactory &copy) :
  BaseParticleFactory(copy) {
  _initial_angle = copy._initial_angle;
  _final_angle = copy._final_angle;
  _initial_angle_spread = copy._initial_angle_spread;
  _final_angle_spread = copy._final_angle_spread;
  _angular_velocity = copy._angular_velocity;
  _angular_velocity_spread = copy._angular_velocity_spread;
  _bUseAngularVelocity = copy._bUseAngularVelocity;
}

////////////////////////////////////////////////////////////////////
//    Function : ~ZSpinParticleFactory
//      Access : virtual, public
// Description : destructor
////////////////////////////////////////////////////////////////////
ZSpinParticleFactory::
~ZSpinParticleFactory() {
}

////////////////////////////////////////////////////////////////////
//    Function : alloc_particle
//      Access : private, virtual
// Description : factory method
////////////////////////////////////////////////////////////////////
BaseParticle *ZSpinParticleFactory::
alloc_particle() const {
  return new ZSpinParticle;
}

////////////////////////////////////////////////////////////////////
//    Function : populate_child_particle
//      Access : private, virtual
// Description : factory populator
////////////////////////////////////////////////////////////////////
void ZSpinParticleFactory::
populate_child_particle(BaseParticle *bp) const {
  ZSpinParticle *zsp = (ZSpinParticle *) bp;

  zsp->set_initial_angle(_initial_angle + SPREAD(_initial_angle_spread));
  zsp->set_final_angle(_final_angle + SPREAD(_final_angle_spread));
  zsp->set_angular_velocity(_angular_velocity + SPREAD(_angular_velocity_spread));
  zsp->enable_angular_velocity(_bUseAngularVelocity);
}
