// Filename: throw_event.I
// Created by:  drose (08Feb99)
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
