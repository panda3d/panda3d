// Filename: physxTriggerHandler.h
// Created by:  pratt (May 16, 2006)
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

#ifndef PHYSXTRIGGERHANDLER_H
#define PHYSXTRIGGERHANDLER_H

#ifdef HAVE_PHYSX

#include "pandabase.h"
#include "pvector.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

class PhysxShape;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxTriggerHandler
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxTriggerHandler : public NxUserTriggerReport {
PUBLISHED:
  void clear_events();
  void throw_events();

public:
  void onTrigger(NxShape &nTriggerShape, NxShape &nOtherShape, NxTriggerFlag status);

private:
  struct PhysxTriggerEvent {
    NxShape *nTriggerShape;
    NxShape *nOtherShape;
    NxTriggerFlag status;
  };

  pvector<PhysxTriggerEvent> events;
};

#include "physxTriggerHandler.I"

#endif // HAVE_PHYSX

#endif // PHYSXTRIGGERHANDLER_H
