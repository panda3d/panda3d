/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_ode.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeWorld.h"
#include "odeMass.h"
#include "odeBody.h"
#include "odeJointGroup.h"
#include "odeJoint.h"
#include "odeSpace.h"
#include "odeGeom.h"
#include "odeSurfaceParameters.h"
#include "odeContactGeom.h"
#include "odeContact.h"
#include "odeAMotorJoint.h"
#include "odeBallJoint.h"
#include "odeContactJoint.h"
#include "odeFixedJoint.h"
#include "odeHingeJoint.h"
#include "odeHinge2Joint.h"
#include "odeLMotorJoint.h"
#include "odeNullJoint.h"
#include "odePlane2dJoint.h"
#include "odeSliderJoint.h"
#include "odeUniversalJoint.h"
#include "odeSimpleSpace.h"
#include "odeHashSpace.h"
#include "odeQuadTreeSpace.h"
#include "odeSphereGeom.h"
#include "odeBoxGeom.h"
#include "odePlaneGeom.h"
#include "odeCappedCylinderGeom.h"
#include "odeCylinderGeom.h"
#include "odeRayGeom.h"
#include "odeTriMeshData.h"
#include "odeTriMeshGeom.h"
#include "odeCollisionEntry.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDAODE)
  #error Buildsystem error: BUILDING_PANDAODE not defined
#endif

Configure(config_ode);
NotifyCategoryDef(ode, "");
NotifyCategoryDef(odeworld, "ode");
NotifyCategoryDef(odebody, "ode");
NotifyCategoryDef(odejoint, "ode");
NotifyCategoryDef(odespace, "ode");
NotifyCategoryDef(odegeom, "ode");
NotifyCategoryDef(odetrimeshdata, "ode");

ConfigureFn(config_ode) {
  init_libode();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libode() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  dInitODE();

  OdeWorld::init_type();
  OdeMass::init_type();
  OdeBody::init_type();
  OdeJointGroup::init_type();
  OdeJoint::init_type();
  OdeSpace::init_type();
  OdeGeom::init_type();
  OdeSurfaceParameters::init_type();
  OdeContactGeom::init_type();
  OdeContact::init_type();
  OdeAMotorJoint::init_type();
  OdeBallJoint::init_type();
  OdeContactJoint::init_type();
  OdeFixedJoint::init_type();
  OdeHingeJoint::init_type();
  OdeHinge2Joint::init_type();
  OdeLMotorJoint::init_type();
  OdeNullJoint::init_type();
  OdePlane2dJoint::init_type();
  OdeSliderJoint::init_type();
  OdeUniversalJoint::init_type();
  OdeSimpleSpace::init_type();
  OdeHashSpace::init_type();
  OdeQuadTreeSpace::init_type();
  OdeSphereGeom::init_type();
  OdeBoxGeom::init_type();
  OdePlaneGeom::init_type();
  OdeCappedCylinderGeom::init_type();
  OdeCylinderGeom::init_type();
  OdeRayGeom::init_type();
  OdeTriMeshData::init_type();
  OdeTriMeshGeom::init_type();
  OdeCollisionEntry::init_type();
}
