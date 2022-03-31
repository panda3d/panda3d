/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file angularEulerIntegrator.h
 * @author charles
 * @date 2000-08-09
 */

#ifndef ANGULAREULERINTEGRATOR_H
#define ANGULAREULERINTEGRATOR_H

#include "angularIntegrator.h"

/**
 * Performs Euler integration on a vector of physically modelable objects
 * given a quantum dt.
 */
class EXPCL_PANDA_PHYSICS AngularEulerIntegrator : public AngularIntegrator {
PUBLISHED:
  AngularEulerIntegrator();
  virtual ~AngularEulerIntegrator();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  virtual void child_integrate(Physical *physical,
                               AngularForceVector& forces,
                               PN_stdfloat dt);
};

#endif // EULERINTEGRATOR_H
