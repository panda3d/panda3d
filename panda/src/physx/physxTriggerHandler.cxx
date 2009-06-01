// Filename: physxTriggerHandler.cxx
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

#ifdef HAVE_PHYSX

#include "physxTriggerHandler.h"

#include "eventParameter.h"
#include "throw_event.h"

////////////////////////////////////////////////////////////////////
//     Function : clear_events
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxTriggerHandler::
clear_events() {
  events.clear();
}

////////////////////////////////////////////////////////////////////
//     Function : throw_events
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxTriggerHandler::
throw_events() {
  pvector<PhysxTriggerEvent>::const_iterator events_iter;
  PhysxTriggerEvent event;
  string on_enter_name = "physxOnTriggerEnter";
  string on_leave_name = "physxOnTriggerLeave";
  string on_stay_name = "physxOnTriggerStay";
  for(events_iter = events.begin(); events_iter != events.end(); ++events_iter) {
    event = *events_iter;
    PT(PhysxShape) pTriggerShape = (PhysxShape *)event.nTriggerShape->userData;
    PT(PhysxShape) pOtherShape = (PhysxShape *)event.nOtherShape->userData;
    if(event.status & NX_TRIGGER_ON_ENTER) {
      throw_event(on_enter_name, EventParameter(pTriggerShape), EventParameter(pOtherShape));
    } else if(event.status & NX_TRIGGER_ON_LEAVE) {
      throw_event(on_leave_name, EventParameter(pTriggerShape), EventParameter(pOtherShape));
    } else if(event.status & NX_TRIGGER_ON_STAY) {
      throw_event(on_stay_name, EventParameter(pTriggerShape), EventParameter(pOtherShape));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function : onTrigger
//       Access : public
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxTriggerHandler::
onTrigger(NxShape &nTriggerShape, NxShape &nOtherShape, NxTriggerFlag status) {
  PhysxTriggerEvent event;
  event.nTriggerShape = &nTriggerShape;
  event.nOtherShape = &nOtherShape;
  event.status = status;
  events.push_back(event);
}

#endif // HAVE_PHYSX
