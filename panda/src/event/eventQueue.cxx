// Filename: eventQueue.cxx
// Created by:  drose (08Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "eventQueue.h"
#include "config_event.h"

EventQueue *EventQueue::_global_event_queue = NULL;


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EventQueue::
EventQueue() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EventQueue::
~EventQueue() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::queue_event
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EventQueue::
queue_event(CPT_Event event) {
  mutex_lock lock(_lock);
  if (_queue.is_full()) {
    event_cat.error()
      << "Ignoring event " << *event << "; event queue full.\n";
  } else {
    _queue.insert(event);
    if (event_cat.is_debug()) {
      event_cat.debug()
	<< "Throwing event " << *event << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::is_queue_empty
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool EventQueue::
is_queue_empty() const {
  return _queue.is_empty();
}


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::dequeue_event
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CPT_Event EventQueue::
dequeue_event() {
  // We need no mutex protection here, as long as there is only one
  // thread extracting events.  The magic of circular buffers.
  return _queue.extract();
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::make_global_event_queue
//       Access: Protected, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void EventQueue::
make_global_event_queue() {
  _global_event_queue = new EventQueue;
}
