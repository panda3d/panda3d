/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeUtil.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODEUTIL_H
#define ODEUTIL_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeJointCollection.h"
#include "odeCollisionEntry.h"

class OdeBody;
class OdeJoint;
class OdeGeom;

/**
 *
 */
class EXPCL_PANDAODE OdeUtil {
PUBLISHED:
  static OdeJoint get_connecting_joint(const OdeBody &body1,
                                       const OdeBody &body2);
  static OdeJointCollection get_connecting_joint_list(const OdeBody &body1,
                                                      const OdeBody &body2);
  static int are_connected(const OdeBody &body1,
                           const OdeBody &body2);
  static int are_connected_excluding(const OdeBody &body1,
                                     const OdeBody &body2,
                                     const int joint_type);
  static PT(OdeCollisionEntry) collide(const OdeGeom &geom1, const OdeGeom &geom2,
                                       const short int max_contacts = 150);

  PY_EXTENSION(static int collide2(const OdeGeom &geom1, const OdeGeom &geom2,
                                   PyObject* arg, PyObject* callback));

  static OdeGeom space_to_geom(const OdeSpace &space);

  static dReal OC_infinity;

  // RAU we can't access OC_infinity as constants are not exposed in python
  static dReal get_infinity() {return OC_infinity;};

  static int rand_get_seed() {return dRandGetSeed();};

  static void rand_set_seed(int s) {dRandSetSeed(s);};
};

#endif
