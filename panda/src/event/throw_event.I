// Filename: throw_event.I
// Created by:  drose (08Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


INLINE void
throw_event(const CPT_Event &event) {
  EventQueue::get_global_event_queue()->queue_event(event);
}

INLINE void
throw_event(const string &event_name) {
  EventQueue::get_global_event_queue()->queue_event(new Event(event_name));
}

INLINE void
throw_event(const string &event_name,
            const EventParameter &p1) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  EventQueue::get_global_event_queue()->queue_event(event);
}

INLINE void
throw_event(const string &event_name,
            const EventParameter &p1,
            const EventParameter &p2) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  event->add_parameter(p2);
  EventQueue::get_global_event_queue()->queue_event(event);
}

INLINE void
throw_event(const string &event_name,
            const EventParameter &p1,
            const EventParameter &p2,
            const EventParameter &p3) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  event->add_parameter(p2);
  event->add_parameter(p3);
  EventQueue::get_global_event_queue()->queue_event(event);
}

////////////////////////////////////////////////////////////////////////////

INLINE void
throw_event_directly(EventHandler& handler,
                     const CPT_Event &event) {
  handler.dispatch_event(event);
}

INLINE void
throw_event_directly(EventHandler& handler,
                     const string &event_name) {
  handler.dispatch_event(new Event(event_name));
}

INLINE void
throw_event_directly(EventHandler& handler,
                     const string &event_name,
                     const EventParameter &p1) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  handler.dispatch_event(event);
}

INLINE void
throw_event_directly(EventHandler& handler,
                     const string &event_name,
                     const EventParameter &p1,
                     const EventParameter &p2) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  event->add_parameter(p2);
  handler.dispatch_event(event);
}

INLINE void
throw_event_directly(EventHandler& handler,
                     const string &event_name,
                     const EventParameter &p1,
                     const EventParameter &p2,
                     const EventParameter &p3) {
  Event *event = new Event(event_name);
  event->add_parameter(p1);
  event->add_parameter(p2);
  event->add_parameter(p3);
  handler.dispatch_event(event);
}
