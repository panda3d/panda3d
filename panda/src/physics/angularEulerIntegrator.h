// Filename: angularEulerIntegrator.h
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
                               float dt);
};

#endif // EULERINTEGRATOR_H
