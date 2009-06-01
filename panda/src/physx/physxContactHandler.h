// Filename: physxContactHandler.h
// Created by:  pratt (May 25, 2006)
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

#ifndef PHYSXCONTACTHANDLER_H
#define PHYSXCONTACTHANDLER_H

#ifdef HAVE_PHYSX

#include "pandabase.h"
#include "pvector.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

class PhysxShape;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxContactHandler
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxContactHandler : public NxUserContactReport {
PUBLISHED:
  PhysxContactHandler();

  void clear_events();
  void throw_events();

  void set_threshold(float value);
  float get_threshold();

public:
  void onContactNotify(NxContactPair& pair, NxU32 flags);

private:
  float threshold;
  struct PhysxContactEvent {
    NxActor *nActor1;
    NxActor *nActor2;
//    string actorName1;
//    string actorName2;
    NxVec3 sumNormalForce;
    NxVec3 sumFrictionForce;
    NxU32 flags;
  };

  pvector<PhysxContactEvent> events;
};

#include "physxContactHandler.I"

#endif // HAVE_PHYSX

#endif // PHYSXCONTACTHANDLER_H
