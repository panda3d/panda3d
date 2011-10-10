// Filename: angularEulerIntegrator.h
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

#ifndef ANGULAREULERINTEGRATOR_H
#define ANGULAREULERINTEGRATOR_H

#include "angularIntegrator.h"

////////////////////////////////////////////////////////////////////
//       Class : AngularEulerIntegrator
// Description : Performs Euler integration on a vector of
//               physically modelable objects given a quantum dt.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS AngularEulerIntegrator : public AngularIntegrator {
PUBLISHED:
  AngularEulerIntegrator();
  virtual ~AngularEulerIntegrator();
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

private:
  virtual void child_integrate(Physical *physical,
                               AngularForceVector& forces,
                               PN_stdfloat dt);
};

#endif // EULERINTEGRATOR_H
