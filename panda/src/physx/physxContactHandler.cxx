// Filename: physxContactHandler.cxx
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

#ifdef HAVE_PHYSX

#include "physxContactHandler.h"

#include "eventParameter.h"
#include "eventStorePandaNode.h"
#include "throw_event.h"

////////////////////////////////////////////////////////////////////
//     Function : PhysxContactHandler
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxContactHandler::
PhysxContactHandler() {
  threshold = 0.01f;
}

////////////////////////////////////////////////////////////////////
//     Function : clear_events
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxContactHandler::
clear_events() {
  events.clear();
}

////////////////////////////////////////////////////////////////////
//     Function : throw_events
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxContactHandler::
throw_events() {
  pvector<PhysxContactEvent>::const_iterator events_iter;
  PhysxContactEvent event;
  string on_start_name = "physxOnContactStart";
  string on_end_name = "physxOnContactEnd";
  string on_touch_name = "physxOnContactTouch";
  for(events_iter = events.begin(); events_iter != events.end(); ++events_iter) {
    event = *events_iter;
    Event *ev = NULL;
    if(event.flags & NX_NOTIFY_ON_START_TOUCH) {
      ev = new Event(on_start_name);
    } else if(event.flags & NX_NOTIFY_ON_END_TOUCH) {
      ev = new Event(on_end_name);
    } else if(event.flags & NX_NOTIFY_ON_TOUCH) {
      ev = new Event(on_touch_name);
    }
    if(ev != NULL) {
      PT(PhysxActorNode) pActorNode1 = (PhysxActorNode *)event.nActor1->userData;
      PT(PhysxActorNode) pActorNode2 = (PhysxActorNode *)event.nActor2->userData;
      ev->add_parameter(EventParameter(new EventStorePandaNode(pActorNode1)));
      ev->add_parameter(EventParameter(new EventStorePandaNode(pActorNode2)));
      ev->add_parameter(EventParameter(event.sumNormalForce.x));
      ev->add_parameter(EventParameter(event.sumNormalForce.y));
      ev->add_parameter(EventParameter(event.sumNormalForce.z));
      ev->add_parameter(EventParameter(event.sumFrictionForce.x));
      ev->add_parameter(EventParameter(event.sumFrictionForce.y));
      ev->add_parameter(EventParameter(event.sumFrictionForce.z));
      EventQueue::get_global_event_queue()->queue_event(ev);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function : set_threshold
//       Access : Published
//  Description : Contacts will only be reported if the magnitude
//                of the normal force squared is greater than this
//                threshold value.
//                Default: 0.01
////////////////////////////////////////////////////////////////////
void PhysxContactHandler::
set_threshold( float value ) {
  threshold = value;
}

////////////////////////////////////////////////////////////////////
//     Function : get_threshold
//       Access : Published
//  Description : Contacts will only be reported if the magnitude
//                of the normal force squared is greater than this
//                threshold value.
//                Default: 0.01
////////////////////////////////////////////////////////////////////
float PhysxContactHandler::
get_threshold() {
  return threshold;
}

////////////////////////////////////////////////////////////////////
//     Function : onContactNotify
//       Access : public
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxContactHandler::
onContactNotify(NxContactPair &pair, NxU32 flags) {
  if(pair.sumNormalForce.magnitudeSquared() >= threshold) {
    PhysxContactEvent event;
    event.nActor1 = pair.actors[0];
    event.nActor2 = pair.actors[1];
//    PhysxActorNode *pActorNode1 = (PhysxActorNode *)pair.actors[0]->userData;
//    PhysxActorNode *pActorNode2 = (PhysxActorNode *)pair.actors[1]->userData;
//    event.actorName1 = pActorNode1->get_name();
//    event.actorName2 = pActorNode2->get_name();
    event.sumNormalForce = pair.sumNormalForce;
    event.sumFrictionForce = pair.sumFrictionForce;
    event.flags = flags;
    events.push_back(event);
  }
}

#endif // HAVE_PHYSX
