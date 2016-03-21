/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxTriggerReport.cxx
 * @author enn0x
 * @date 2009-09-19
 */

#include "physxTriggerReport.h"

#include "event.h"
#include "eventQueue.h"
#include "eventParameter.h"

PStatCollector PhysxTriggerReport::_pcollector("App:PhysX:Trigger Reporting");

/**
 *
 */
void PhysxTriggerReport::
enable() {

  _enabled = true;
}

/**
 *
 */
void PhysxTriggerReport::
disable() {

  _enabled = false;
}

/**
 *
 */
bool PhysxTriggerReport::
is_enabled() const {

  return _enabled;
}

/**
 *
 */
void PhysxTriggerReport::
onTrigger(NxShape &triggerShape, NxShape &otherShape, NxTriggerFlag status) {

  if (!_enabled) {
    return;
  }

  _pcollector.start();

  Event *event;
  if (status & NX_TRIGGER_ON_ENTER) {
    event = new Event("physx-trigger-enter");
  }
  else if (status & NX_TRIGGER_ON_LEAVE) {
    event = new Event("physx-trigger-leave");
  }
  else if (status & NX_TRIGGER_ON_STAY) {
    event = new Event("physx-trigger-stay");
  }
  else {
    return;
  }

  PT(PhysxShape) pTriggerShape = (PhysxShape *)triggerShape.userData;
  PT(PhysxShape) pOtherShape = (PhysxShape *)otherShape.userData;
  event->add_parameter(EventParameter(pTriggerShape));
  event->add_parameter(EventParameter(pOtherShape));

  EventQueue::get_global_event_queue()->queue_event(event);

  _pcollector.stop();
}
