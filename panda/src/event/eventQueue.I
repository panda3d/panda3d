// Filename: eventQueue.I
// Created by:  drose (05May00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::get_global_event_queue
//       Access: Public
//  Description: Returns a pointer to the one global EventQueue
//               object.  If the global object has not yet been
//               created, this will create it.
////////////////////////////////////////////////////////////////////
INLINE EventQueue *EventQueue::
get_global_event_queue() {
  if (_global_event_queue == NULL) {
    make_global_event_queue();
  }
  return _global_event_queue;
}
