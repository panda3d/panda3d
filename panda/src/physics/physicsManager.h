/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physicsManager.h
 * @author charles
 * @date 2000-06-14
 */

#ifndef PHYSICSMANAGER_H
#define PHYSICSMANAGER_H

#include "pandabase.h"
#include "pointerTo.h"

#include "physical.h"
#include "linearForce.h"
#include "angularForce.h"
#include "linearIntegrator.h"
#include "angularIntegrator.h"
#include "physicalNode.h"

#include "plist.h"
#include "pvector.h"

#include "configVariableInt.h"

/**
 * Physics don't get much higher-level than this.  Attach as many Physicals
 * (particle systems, etc..) as you want, pick an integrator and go.
 */
class EXPCL_PANDA_PHYSICS PhysicsManager {
public:
  // NOTE that the physicals container is NOT reference counted.  this does
  // indeed mean that you are NOT supposed to use this as a primary storage
  // container for the physicals.  This is so because physicals, on their
  // death, ask to be removed from their current physicsmanager, if one
  // exists, relieving the client from the task and also allowing for
  // dynamically created and destroyed physicals.
  typedef pvector<Physical *> PhysicalsVector;
  typedef pvector<PT(LinearForce)> LinearForceVector;
  typedef pvector<PT(AngularForce)> AngularForceVector;

PUBLISHED:
  PhysicsManager();
  virtual ~PhysicsManager();

  INLINE void attach_linear_integrator(LinearIntegrator *i);
  INLINE void attach_angular_integrator(AngularIntegrator *i);
  INLINE void attach_physical(Physical *p);
  INLINE void attach_physicalnode(PhysicalNode *p); // use attach_physical_node instead.
  INLINE void attach_physical_node(PhysicalNode *p);
  INLINE void add_linear_force(LinearForce *f);
  INLINE void add_angular_force(AngularForce *f);
  INLINE void clear_linear_forces();
  INLINE void clear_angular_forces();
  INLINE void clear_physicals();

  INLINE void set_viscosity(PN_stdfloat viscosity);
  INLINE PN_stdfloat get_viscosity() const;

  void remove_physical(Physical *p);
  void remove_physical_node(PhysicalNode *p);
  void remove_linear_force(LinearForce *f);
  void remove_angular_force(AngularForce *f);
  void do_physics(PN_stdfloat dt);
  void do_physics(PN_stdfloat dt, Physical *p);
  void init_random_seed();

  virtual void output(std::ostream &out) const;
  virtual void write_physicals(std::ostream &out, int indent=0) const;
  virtual void write_linear_forces(std::ostream &out, int indent=0) const;
  virtual void write_angular_forces(std::ostream &out, int indent=0) const;
  virtual void write(std::ostream &out, int indent=0) const;

  virtual void debug_output(std::ostream &out, int indent=0) const;

public:
  friend class Physical;
  static ConfigVariableInt _random_seed;

private:
  PN_stdfloat _viscosity;
  PhysicalsVector _physicals;
  LinearForceVector _linear_forces;
  AngularForceVector _angular_forces;

  PT(LinearIntegrator) _linear_integrator;
  PT(AngularIntegrator) _angular_integrator;
};

#include "physicsManager.I"

#endif // PHYSICSMANAGER_H
