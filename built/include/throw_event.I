/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file throw_event.I
 * @author drose
 * @date 1999-02-08
 */

INLINE void
throw_event(const CPT_Event &event) {
  EventQueue::get_global_event_queue()->queue_event(event);
}

INLINE void
throw_event(const std::string &event_name) {
  EventQueue::get_global_event_queue()->queue_event(new Event(event_name));
}

INLINE void
throw_event(const std::string &event_name,
            const EventParameter &p1) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  EventQueue::get_global_event_queue()->queue_event(event);
}

INLINE void
throw_event(const std::string &event_name,
            const EventParameter &p1,
            const EventParameter &p2) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  event->add_parameter(p2);
  EventQueue::get_global_event_queue()->queue_event(event);
}

INLINE void
throw_event(const std::string &event_name,
            const EventParameter &p1,
            const EventParameter &p2,
            const EventParameter &p3) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  event->add_parameter(p2);
  event->add_parameter(p3);
  EventQueue::get_global_event_queue()->queue_event(event);
}

INLINE void
throw_event(const std::string &event_name,
            const EventParameter &p1,
            const EventParameter &p2,
            const EventParameter &p3,
            const EventParameter &p4) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  event->add_parameter(p2);
  event->add_parameter(p3);
  event->add_parameter(p4);
  EventQueue::get_global_event_queue()->queue_event(event);
}


INLINE void
throw_event_directly(EventHandler& handler,
                     const CPT_Event &event) {
  handler.dispatch_event(event);
}

INLINE void
throw_event_directly(EventHandler& handler,
                     const std::string &event_name) {
  handler.dispatch_event(new Event(event_name));
}

INLINE void
throw_event_directly(EventHandler& handler,
                     const std::string &event_name,
                     const EventParameter &p1) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  handler.dispatch_event(event);
}

INLINE void
throw_event_directly(EventHandler& handler,
                     const std::string &event_name,
                     const EventParameter &p1,
                     const EventParameter &p2) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  event->add_parameter(p2);
  handler.dispatch_event(event);
}

INLINE void
throw_event_directly(EventHandler& handler,
                     const std::string &event_name,
                     const EventParameter &p1,
                     const EventParameter &p2,
                     const EventParameter &p3) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  event->add_parameter(p2);
  event->add_parameter(p3);
  handler.dispatch_event(event);
}
