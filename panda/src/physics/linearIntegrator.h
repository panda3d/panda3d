// Filename: linearIntegrator.h
// Created by:  charles (13Jun00)
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

#ifndef LINEARINTEGRATOR_H
#define LINEARINTEGRATOR_H

#include "physicsObject.h"
#include "baseIntegrator.h"
#include "linearForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearIntegrator
// Description : Pure virtual base class for physical modeling.
//               Takes physically modelable objects and applies
//               forces to them.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearIntegrator : public BaseIntegrator {
public:
  virtual ~LinearIntegrator();

  void integrate(Physical *physical, pvector< PT(LinearForce) > &forces,
                 float dt);
  
  virtual void output(ostream &out, unsigned int indent=0) const;

protected:
  LinearIntegrator();

private:
  static const float _max_linear_dt;

  // this allows baseLinearIntegrator to censor/modify data that the
  // actual integration function receives.
  virtual void child_integrate(Physical *physical, 
                               pvector< PT(LinearForce) > &forces,
                               float dt) = 0;
};

#endif // LINEARINTEGRATOR_H
