/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearIntegrator.h
 * @author charles
 * @date 2000-06-13
 */

#ifndef LINEARINTEGRATOR_H
#define LINEARINTEGRATOR_H

#include "physicsObject.h"
#include "baseIntegrator.h"
#include "linearForce.h"
#include "configVariableDouble.h"

/**
 * Pure virtual base class for physical modeling.  Takes physically modelable
 * objects and applies forces to them.
 */
class EXPCL_PANDA_PHYSICS LinearIntegrator : public BaseIntegrator {
PUBLISHED:
  virtual ~LinearIntegrator();
public:

  void integrate(Physical *physical, LinearForceVector &forces,
                 PN_stdfloat dt);

PUBLISHED:
  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

protected:
  LinearIntegrator();

private:
  static ConfigVariableDouble _max_linear_dt;

  // this allows baseLinearIntegrator to censormodify data that the actual
  // integration function receives.
  virtual void child_integrate(Physical *physical,
                               LinearForceVector &forces,
                               PN_stdfloat dt) = 0;
};

#endif // LINEARINTEGRATOR_H
