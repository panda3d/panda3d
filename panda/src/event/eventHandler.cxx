// Filename: eventHandler.cxx
// Created by:  drose (08Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "eventHandler.h"
#include "eventQueue.h"
#include "config_event.h"

TypeHandle EventHandler::_type_handle;

EventHandler *EventHandler::_global_event_handler = 0;


////////////////////////////////////////////////////////////////////
//     Function: EventHandler::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EventHandler::
EventHandler(EventQueue *queue) : _queue(*queue) {
}

////////////////////////////////////////////////////////////////////
//     Function: EventHandler::process_events
//       Access: Public
//  Description: The main processing loop of the EventHandler.  This
//               function must be called periodically to service
//               events.  Walks through each pending event and calls
//               its assigned hooks.
////////////////////////////////////////////////////////////////////
void EventHandler::
process_events() {
  while (!_queue.is_queue_empty()) {
    dispatch_event(_queue.dequeue_event());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EventHandler::dispatch_event
//       Access: Public, Virtual
//  Description: Calls the hooks assigned to the indicated single
//               event.
////////////////////////////////////////////////////////////////////
void EventHandler::
dispatch_event(const CPT_Event &event) {
  nassertv(!event.is_null());
  // Is the event name defined in the hook table?  It will be if
  // anyone has ever assigned a hook to this particular event name.
  Hooks::const_iterator hi;
  hi = _hooks.find(event->get_name());

  if (hi != _hooks.end()) {
    // Yes, it is!  Now walk through all the functions assigned to
    // that event name.
    Functions copy_functions = (*hi).second;

    Functions::const_iterator fi;
    for (fi = copy_functions.begin(); fi != copy_functions.end(); ++fi) {
      if (event_cat.is_spam()) {
        event_cat->spam() << "calling callback 0x" << (void*)(*fi)
              << " for event '" << event->get_name() << "'"
              << endl;
      }
      (*fi)(event);
    }
  }

  // now for callback hooks
  CallbackHooks::const_iterator cbhi;
  cbhi = _cbhooks.find(event->get_name());

  if (cbhi != _cbhooks.end()) {
    // found one
    CallbackFunctions copy_functions = (*cbhi).second;

    CallbackFunctions::const_iterator cbfi;
    for (cbfi = copy_functions.begin(); cbfi != copy_functions.end(); ++cbfi) {
      ((*cbfi).first)(event, (*cbfi).second);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EventHandler::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EventHandler::
write(ostream &out) const {
  Hooks::const_iterator hi;
  hi = _hooks.begin();

  CallbackHooks::const_iterator cbhi;
  cbhi = _cbhooks.begin();

  while (hi != _hooks.end() && cbhi != _cbhooks.end()) {
    if ((*hi).first < (*cbhi).first) {
      write_hook(out, *hi);
      ++hi;
    } else if ((*cbhi).first < (*hi).first) {
      write_cbhook(out, *cbhi);
      ++cbhi;
    } else {
      write_hook(out, *hi);
      write_cbhook(out, *cbhi);
      ++hi;
      ++cbhi;
    }
  }

  while (hi != _hooks.end()) {
    write_hook(out, *hi);
    ++hi;
  }

  while (cbhi != _cbhooks.end()) {
    write_cbhook(out, *cbhi);
    ++cbhi;
  }
}



////////////////////////////////////////////////////////////////////
//     Function: EventHandler::add_hook
//       Access: Public
//  Description: Adds the indicated function to the list of those that
//               will be called when the named event is thrown.
//               Returns true if the function was successfully added,
//               false if it was already defined on the indicated
//               event name.
////////////////////////////////////////////////////////////////////
bool EventHandler::
add_hook(const string &event_name, EventFunction *function) {
  if (event_cat.is_debug()) {
    event_cat.debug() << "adding hook for event '" << event_name
               << "' with function 0x" << (void*)function << endl;
  }
  assert(!event_name.empty());
  assert(function);
  return _hooks[event_name].insert(function).second;
}


////////////////////////////////////////////////////////////////////
//     Function: EventHandler::add_hook
//       Access: Public
//  Description: Adds the indicated function to the list of those that
//               will be called when the named event is thrown.
//               Returns true if the function was successfully added,
//               false if it was already defined on the indicated
//               event name.  This version records an untyped pointer
//               to user callback data.
////////////////////////////////////////////////////////////////////
bool EventHandler::
add_hook(const string &event_name, EventCallbackFunction *function,
         void *data) {
  assert(!event_name.empty());
  assert(function);
  return _cbhooks[event_name].insert(CallbackFunction(function, data)).second;
}

////////////////////////////////////////////////////////////////////
//     Function: EventHandler::has_hook
//       Access: Public
//  Description: Returns true if there is any hook added on the
//               indicated event name, false otherwise.
////////////////////////////////////////////////////////////////////
bool EventHandler::
has_hook(const string &event_name) const {
  assert(!event_name.empty());
  Hooks::const_iterator hi;
  hi = _hooks.find(event_name);
  if (hi != _hooks.end()) {
    if (!(*hi).second.empty()) {
      return true;
    }
  }

  CallbackHooks::const_iterator chi;
  chi = _cbhooks.find(event_name);
  if (chi != _cbhooks.end()) {
    if (!(*chi).second.empty()) {
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: EventHandler::remove_hook
//       Access: Public
//  Description: Removes the indicated function from the named event
//               hook.  Returns true if the hook was removed, false if
//               it wasn't there in the first place.
////////////////////////////////////////////////////////////////////
bool EventHandler::
remove_hook(const string &event_name, EventFunction *function) {
  assert(!event_name.empty());
  assert(function);
  return _hooks[event_name].erase(function) != 0;
}


////////////////////////////////////////////////////////////////////
//     Function: EventHandler::remove_hook
//       Access: Public
//  Description: Removes the indicated function from the named event
//               hook.  Returns true if the hook was removed, false if
//               it wasn't there in the first place.  This version
//               takes an untyped pointer to user callback data.
////////////////////////////////////////////////////////////////////
bool EventHandler::
remove_hook(const string &event_name, EventCallbackFunction *function,
            void *data) {
  assert(!event_name.empty());
  assert(function);
  return _cbhooks[event_name].erase(CallbackFunction(function, data)) != 0;
}


////////////////////////////////////////////////////////////////////
//     Function: EventHandler::remove_all_hooks
//       Access: Public
//  Description: Removes all hooks assigned to all events.
////////////////////////////////////////////////////////////////////
void EventHandler::
remove_all_hooks() {
  _hooks.clear();
  _cbhooks.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: EventHandler::make_global_event_handler
//       Access: Protected, Static
//  Description:
////////////////////////////////////////////////////////////////////
void EventHandler::
make_global_event_handler(EventQueue *queue) {
  assert(queue);
  _global_event_handler = new EventHandler(queue);
}


////////////////////////////////////////////////////////////////////
//     Function: EventHandler::write_hook
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void EventHandler::
write_hook(ostream &out, const EventHandler::Hooks::value_type &hook) const {
  if (!hook.second.empty()) {
    out << hook.first << " has " << hook.second.size() << " functions.\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EventHandler::write_cbhook
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void EventHandler::
write_cbhook(ostream &out, const EventHandler::CallbackHooks::value_type &hook) const {
  if (!hook.second.empty()) {
    out << hook.first << " has " << hook.second.size() << " callback functions.\n";
  }
}
