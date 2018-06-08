/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventHandler.h
 * @author drose
 * @date 1999-02-08
 */

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "pandabase.h"

#include "event.h"
#include "pt_Event.h"
#include "asyncFuture.h"

#include "pset.h"
#include "pmap.h"

class EventQueue;

/**
 * A class to monitor events from the C++ side of things.  It maintains a set
 * of "hooks", function pointers assigned to event names, and calls the
 * appropriate hooks when the matching event is detected.
 *
 * This class is not necessary when the hooks are detected and processed
 * entirely by the scripting language, e.g.  via Scheme hooks or the messenger
 * in Python.
 */
class EXPCL_PANDA_EVENT EventHandler : public TypedObject {
public:
  // Define a function type suitable for receiving events.
  typedef void EventFunction(const Event *);
  typedef void EventCallbackFunction(const Event *, void *);

PUBLISHED:
  explicit EventHandler(EventQueue *ev_queue);
  ~EventHandler() {}

  AsyncFuture *get_future(const std::string &event_name);

  void process_events();

  virtual void dispatch_event(const Event *event);

  void write(std::ostream &out) const;

  INLINE static EventHandler *get_global_event_handler(EventQueue *queue = nullptr);

public:
  bool add_hook(const std::string &event_name, EventFunction *function);
  bool add_hook(const std::string &event_name, EventCallbackFunction *function,
                void *data);
  bool has_hook(const std::string &event_name) const;
  bool has_hook(const std::string &event_name, EventFunction *function) const;
  bool has_hook(const std::string &event_name, EventCallbackFunction *function,
                void *data) const;
  bool remove_hook(const std::string &event_name, EventFunction *function);
  bool remove_hook(const std::string &event_name, EventCallbackFunction *function,
                   void *data);

  bool remove_hooks(const std::string &event_name);
  bool remove_hooks_with(void *data);

  void remove_all_hooks();

protected:

  typedef pset<EventFunction *> Functions;
  typedef pmap<std::string, Functions> Hooks;
  typedef std::pair<EventCallbackFunction*, void*> CallbackFunction;
  typedef pset<CallbackFunction> CallbackFunctions;
  typedef pmap<std::string, CallbackFunctions> CallbackHooks;
  typedef pmap<std::string, PT(AsyncFuture)> Futures;

  Hooks _hooks;
  CallbackHooks _cbhooks;
  Futures _futures;
  EventQueue &_queue;

  static EventHandler *_global_event_handler;
  static void make_global_event_handler();

private:
  void write_hook(std::ostream &out, const Hooks::value_type &hook) const;
  void write_cbhook(std::ostream &out, const CallbackHooks::value_type &hook) const;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "EventHandler",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eventHandler.I"

#endif
