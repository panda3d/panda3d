// Filename: physxJointHandler.h
// Created by:  pratt (Jul 9, 2006)
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

#ifndef PHYSXJOINTHANDLER_H
#define PHYSXJOINTHANDLER_H

#ifdef HAVE_PHYSX

#include "pandabase.h"
#include "pvector.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

class PhysxJoint;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxJointHandler
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxJointHandler : public NxUserNotify {
PUBLISHED:
  void clear_events();
  void throw_events();

public:
  bool onJointBreak(NxReal breakingForce, NxJoint &brokenJoint);

  // currently not implemented; need to rework the whole reporting interface
  void onWake(NxActor **actors, NxU32 count);
  void onSleep(NxActor **actors, NxU32 count);

private:
  struct PhysxJointEvent {
    NxReal breakingForce;
    NxJoint *brokenJoint;
  };

  pvector<PhysxJointEvent> events;
};

#include "physxJointHandler.I"

#endif // HAVE_PHYSX

#endif // PHYSXJOINTHANDLER_H
