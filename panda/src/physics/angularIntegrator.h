// Filename: angularIntegrator.h
// Created by:  charles (09Aug00)
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

#ifndef ANGULARINTEGRATOR_H
#define ANGULARINTEGRATOR_H

#include "baseIntegrator.h"
#include "angularForce.h"
#include "configVariableDouble.h"

////////////////////////////////////////////////////////////////////
//       Class : BaseAngularIntegrator
// Description : Pure virtual base class for physical modeling.
//               Takes physically modelable objects and applies
//               forces to them.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS AngularIntegrator : public BaseIntegrator {
PUBLISHED:
  virtual ~AngularIntegrator();
public:

  void integrate(Physical *physical, AngularForceVector &forces,
                 PN_stdfloat dt);

PUBLISHED:  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

protected:
  AngularIntegrator();

private:
  static ConfigVariableDouble _max_angular_dt;

  // this allows baseAngularIntegrator to censor/modify data that the
  // actual integration function receives.
  virtual void child_integrate(Physical *physical, AngularForceVector &forces,
                               PN_stdfloat dt) = 0;
};

#endif // ANGULARINTEGRATOR_H
