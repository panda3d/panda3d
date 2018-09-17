/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventHandler.cxx
 * @author drose
 * @date 1999-02-08
 */

#include "eventHandler.h"
#include "eventQueue.h"
#include "config_event.h"

using std::string;

TypeHandle EventHandler::_type_handle;

EventHandler *EventHandler::_global_event_handler = nullptr;


/**
 *
 */
EventHandler::
EventHandler(EventQueue *ev_queue) : _queue(*ev_queue) {
}

/**
 * Returns a pending future that will be marked as done when the event is next
 * fired.
 */
AsyncFuture *EventHandler::
get_future(const string &event_name) {
  Futures::iterator fi;
  fi = _futures.find(event_name);

  // If we already have a future, but someone cancelled it, we need to create
  // a new future instead.
  if (fi != _futures.end() && !fi->second->cancelled()) {
    return fi->second;
  } else {
    AsyncFuture *fut = new AsyncFuture;
    _futures[event_name] = fut;
    return fut;
  }
}

/**
 * The main processing loop of the EventHandler.  This function must be called
 * periodically to service events.  Walks through each pending event and calls
 * its assigned hooks.
 */
void EventHandler::
process_events() {
  while (!_queue.is_queue_empty()) {
    dispatch_event(_queue.dequeue_event());
  }
}

/**
 * Calls the hooks assigned to the indicated single event.
 */
void EventHandler::
dispatch_event(const Event *event) {
  nassertv(event != nullptr);

  // Is the event name defined in the hook table?  It will be if anyone has
  // ever assigned a hook to this particular event name.
  Hooks::const_iterator hi;
  hi = _hooks.find(event->get_name());

  if (hi != _hooks.end()) {
    // Yes, it is!  Now walk through all the functions assigned to that event
    // name.
    Functions copy_functions = (*hi).second;

    Functions::const_iterator fi;
    for (fi = copy_functions.begin(); fi != copy_functions.end(); ++fi) {
      if (event_cat.is_spam()) {
        event_cat->spam()
          << "calling callback 0x" << (void*)(*fi)
          << " for event '" << event->get_name() << "'"
          << std::endl;
      }
      (*fi)(event);
    }
  }

  // now for callback hooks
  CallbackHooks::const_iterator chi;
  chi = _cbhooks.find(event->get_name());

  if (chi != _cbhooks.end()) {
    // found one
    CallbackFunctions copy_functions = (*chi).second;

    CallbackFunctions::const_iterator cfi;
    for (cfi = copy_functions.begin(); cfi != copy_functions.end(); ++cfi) {
      ((*cfi).first)(event, (*cfi).second);
    }
  }

  // Finally, check for futures that need to be triggered.
  Futures::iterator fi;
  fi = _futures.find(event->get_name());

  if (fi != _futures.end()) {
    AsyncFuture *fut = (*fi).second;
    if (!fut->done()) {
      fut->set_result((TypedReferenceCount *)event);
    }
    _futures.erase(fi);
  }
}


/**
 *
 */
void EventHandler::
write(std::ostream &out) const {
  Hooks::const_iterator hi;
  hi = _hooks.begin();

  CallbackHooks::const_iterator chi;
  chi = _cbhooks.begin();

  while (hi != _hooks.end() && chi != _cbhooks.end()) {
    if ((*hi).first < (*chi).first) {
      write_hook(out, *hi);
      ++hi;
    } else if ((*chi).first < (*hi).first) {
      write_cbhook(out, *chi);
      ++chi;
    } else {
      write_hook(out, *hi);
      write_cbhook(out, *chi);
      ++hi;
      ++chi;
    }
  }

  while (hi != _hooks.end()) {
    write_hook(out, *hi);
    ++hi;
  }

  while (chi != _cbhooks.end()) {
    write_cbhook(out, *chi);
    ++chi;
  }
}



/**
 * Adds the indicated function to the list of those that will be called when
 * the named event is thrown.  Returns true if the function was successfully
 * added, false if it was already defined on the indicated event name.
 */
bool EventHandler::
add_hook(const string &event_name, EventFunction *function) {
  if (event_cat.is_debug()) {
    event_cat.debug()
      << "adding hook for event '" << event_name
      << "' with function 0x" << (void*)function << std::endl;
  }
  assert(!event_name.empty());
  assert(function);
  return _hooks[event_name].insert(function).second;
}


/**
 * Adds the indicated function to the list of those that will be called when
 * the named event is thrown.  Returns true if the function was successfully
 * added, false if it was already defined on the indicated event name.  This
 * version records an untyped pointer to user callback data.
 */
bool EventHandler::
add_hook(const string &event_name, EventCallbackFunction *function,
         void *data) {
  assert(!event_name.empty());
  assert(function);
  return _cbhooks[event_name].insert(CallbackFunction(function, data)).second;
}

/**
 * Returns true if there is any hook added on the indicated event name, false
 * otherwise.
 */
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


/**
 * Returns true if there is the hook added on the indicated event name and
 * function pointer, false otherwise.
 */
bool EventHandler::
has_hook(const string &event_name, EventFunction *function) const {
  assert(!event_name.empty());
  Hooks::const_iterator hi;
  hi = _hooks.find(event_name);
  if (hi != _hooks.end()) {
    const Functions& functions = (*hi).second;
    if (functions.find(function) != functions.end()) {
      return true;
    }
  }

  return false;
}


/**
 * Returns true if there is the hook added on the indicated event name,
 * function pointer and callback data, false otherwise.
 */
bool EventHandler::
has_hook(const string &event_name, EventCallbackFunction *function, void *data) const {
  assert(!event_name.empty());
  CallbackHooks::const_iterator chi;
  chi = _cbhooks.find(event_name);
  if (chi != _cbhooks.end()) {
    const CallbackFunctions& cbfunctions = (*chi).second;
    if (cbfunctions.find(CallbackFunction(function, data)) != cbfunctions.end()) {
      return true;
    }
  }

  return false;
}


/**
 * Removes the indicated function from the named event hook.  Returns true if
 * the hook was removed, false if it wasn't there in the first place.
 */
bool EventHandler::
remove_hook(const string &event_name, EventFunction *function) {
  assert(!event_name.empty());
  assert(function);
  return _hooks[event_name].erase(function) != 0;
}


/**
 * Removes the indicated function from the named event hook.  Returns true if
 * the hook was removed, false if it wasn't there in the first place.  This
 * version takes an untyped pointer to user callback data.
 */
bool EventHandler::
remove_hook(const string &event_name, EventCallbackFunction *function,
            void *data) {
  assert(!event_name.empty());
  assert(function);
  return _cbhooks[event_name].erase(CallbackFunction(function, data)) != 0;
}

/**
 * Removes all functions from the named event hook.  Returns true if any
 * functions were removed, false if there were no functions added to the hook.
 */
bool EventHandler::
remove_hooks(const string &event_name) {
  assert(!event_name.empty());
  bool any_removed = false;

  Hooks::iterator hi = _hooks.find(event_name);
  if (hi != _hooks.end()) {
    if (!(*hi).second.empty()) {
      any_removed = true;
    }
    _hooks.erase(hi);
  }

  CallbackHooks::iterator chi = _cbhooks.find(event_name);
  if (chi != _cbhooks.end()) {
    if (!(*chi).second.empty()) {
      any_removed = true;
    }
    _cbhooks.erase(chi);
  }

  return any_removed;
}

/**
 * Removes all CallbackFunction hooks that have the indicated pointer as the
 * associated data pointer.
 */
bool EventHandler::
remove_hooks_with(void *data) {
  bool any_removed = false;

  CallbackHooks::iterator chi;
  for (chi = _cbhooks.begin(); chi != _cbhooks.end(); ++chi) {
    CallbackFunctions &funcs = (*chi).second;
    CallbackFunctions::iterator cfi;

    CallbackFunctions new_funcs;
    for (cfi = funcs.begin(); cfi != funcs.end(); ++cfi) {
      if ((*cfi).second == data) {
        any_removed = true;
      } else {
        new_funcs.insert(*cfi);
      }
    }
    funcs.swap(new_funcs);
  }

  return any_removed;
}


/**
 * Removes all hooks assigned to all events.
 */
void EventHandler::
remove_all_hooks() {
  _hooks.clear();
  _cbhooks.clear();
}

/**
 *
 */
void EventHandler::
make_global_event_handler() {
  init_memory_hook();
  _global_event_handler = new EventHandler(EventQueue::get_global_event_queue());
}


/**
 *
 */
void EventHandler::
write_hook(std::ostream &out, const EventHandler::Hooks::value_type &hook) const {
  if (!hook.second.empty()) {
    out << hook.first << " has " << hook.second.size() << " functions.\n";
  }
}

/**
 *
 */
void EventHandler::
write_cbhook(std::ostream &out, const EventHandler::CallbackHooks::value_type &hook) const {
  if (!hook.second.empty()) {
    out << hook.first << " has " << hook.second.size() << " callback functions.\n";
  }
}
