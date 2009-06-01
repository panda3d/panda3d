// Filename: config_physics.cxx
// Created by:  pratt (Apr 18, 2006)
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

#include "config_physx.h"

#include "dconfig.h"

#include "physxActorNode.h"
#include "physxJoint.h"
#include "physxD6Joint.h"
#include "physxShape.h"
#include "physxBoxShape.h"
#include "physxCapsuleShape.h"
#include "physxPlaneShape.h"
#include "physxSphereShape.h"

ConfigureDef(config_physx);
NotifyCategoryDef(physx, "");

ConfigureFn(config_physx) {
  init_libphysx();
}

ConfigVariableBool physx_want_visual_debugger
("physx-want-visual-debugger", false);

ConfigVariableString physx_visual_debugger_host
("physx-visual-debugger-host", "localhost");

ConfigVariableInt physx_visual_debugger_port
("physx-visual-debugger-port", 5425);


////////////////////////////////////////////////////////////////////
//     Function: init_libphysx
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libphysx() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  PhysxActorNode::init_type();
  PhysxJoint::init_type();
  PhysxD6Joint::init_type();
  PhysxShape::init_type();
  PhysxBoxShape::init_type();
  PhysxCapsuleShape::init_type();
  PhysxPlaneShape::init_type();
  PhysxSphereShape::init_type();
}
