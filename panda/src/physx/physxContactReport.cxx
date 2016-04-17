/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxContactReport.cxx
 * @author enn0x
 * @date 2009-09-19
 */

#include "physxContactReport.h"
#include "physxContactPair.h"
#include "physxManager.h"

#include "event.h"
#include "eventQueue.h"
#include "eventParameter.h"

PStatCollector PhysxContactReport::_pcollector("App:PhysX:Contact Reporting");

/**
 *
 */
void PhysxContactReport::
enable() {

  _enabled = true;
}

/**
 *
 */
void PhysxContactReport::
disable() {

  _enabled = false;
}

/**
 *
 */
bool PhysxContactReport::
is_enabled() const {

  return _enabled;
}

/**
 *
 */
void PhysxContactReport::
onContactNotify(NxContactPair &pair, NxU32 flags) {

  if (!_enabled) {
    return;
  }

  _pcollector.start();

  Event *event;
  if (flags & NX_NOTIFY_ON_START_TOUCH) {
    event = new Event("physx-contact-start");
  }
  else if (flags & NX_NOTIFY_ON_END_TOUCH) {
    event = new Event("physx-contact-stop");
  }
  else if (flags & NX_NOTIFY_ON_TOUCH) {
    event = new Event("physx-contact-touch");
  }
  else if (flags & NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD) {
    event = new Event("physx-contact-start-force-threshold");
  }
  else if (flags & NX_NOTIFY_ON_END_TOUCH_FORCE_THRESHOLD) {
    event = new Event("physx-contact-stop-force-threshold");
  }
  else if (flags & NX_NOTIFY_ON_TOUCH_FORCE_THRESHOLD) {
    event = new Event("physx-contact-touch-force-threshold");
  }
  else {
    return;
  }

  PT(PhysxContactPair) ppair = new PhysxContactPair(pair);
  event->add_parameter(EventParameter(ppair));
  EventQueue::get_global_event_queue()->queue_event(event);

  _pcollector.stop();
}
