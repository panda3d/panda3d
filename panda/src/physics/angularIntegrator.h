// Filename: angularIntegrator.h
// Created by:  charles (09Aug00)
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

#ifndef ANGULARINTEGRATOR_H
#define ANGULARINTEGRATOR_H

#include "baseIntegrator.h"
#include "angularForce.h"

////////////////////////////////////////////////////////////////////
//       Class : BaseAngularIntegrator
// Description : Pure virtual base class for physical modeling.
//               Takes physically modelable objects and applies
//               forces to them.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS AngularIntegrator : public BaseIntegrator {
public:
  virtual ~AngularIntegrator();

  void integrate(Physical *physical, AngularForceVector &forces,
                 float dt);
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

protected:
  AngularIntegrator();

private:
  static const float _max_angular_dt;

  // this allows baseAngularIntegrator to censor/modify data that the
  // actual integration function receives.
  virtual void child_integrate(Physical *physical, AngularForceVector &forces,
                               float dt) = 0;
};

#endif // ANGULARINTEGRATOR_H
