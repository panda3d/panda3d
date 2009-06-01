// Filename: physxJointHandler.cxx
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

#ifdef HAVE_PHYSX

#include "physxJointHandler.h"

#include "eventParameter.h"
#include "throw_event.h"

////////////////////////////////////////////////////////////////////
//     Function : clear_events
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointHandler::
clear_events() {
  events.clear();
}

////////////////////////////////////////////////////////////////////
//     Function : throw_events
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointHandler::
throw_events() {
  pvector<PhysxJointEvent>::const_iterator events_iter;
  PhysxJointEvent event;
  for(events_iter = events.begin(); events_iter != events.end(); ++events_iter) {
    event = *events_iter;
    PT(PhysxJoint) pJoint = (PhysxJoint *)(event.brokenJoint->userData);
    throw_event("physxOnJointBreak", EventParameter(event.breakingForce), EventParameter(pJoint));
  }
}

////////////////////////////////////////////////////////////////////
//     Function : onJointBreak
//       Access : public
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxJointHandler::
onJointBreak(NxReal breakingForce, NxJoint &brokenJoint) {
  PhysxJointEvent event;
  event.breakingForce = breakingForce;
  event.brokenJoint = &brokenJoint;
  events.push_back(event);

  PhysxJoint *pJoint = (PhysxJoint *)event.brokenJoint->userData;
  return false;  // Do not release the joint
}

////////////////////////////////////////////////////////////////////
//     Function : onWake
//       Access : public
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointHandler::
onWake(NxActor **actors, NxU32 count) {
  // currently not implemented; need to rework the whole reporting interface
}

////////////////////////////////////////////////////////////////////
//     Function : onSleep
//       Access : public
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointHandler::
onSleep(NxActor **actors, NxU32 count) {
  // currently not implemented; need to rework the whole reporting interface
}

#endif // HAVE_PHYSX
