// Filename: eventHandler.h
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

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "pandabase.h"

#include "event.h"
#include "pt_Event.h"

#include "pset.h"
#include "pmap.h"

class EventQueue;

////////////////////////////////////////////////////////////////////
//       Class : EventHandler
// Description : A class to monitor events from the C++ side of
//               things.  It maintains a set of "hooks", function
//               pointers assigned to event names, and calls the
//               appropriate hooks when the matching event is
//               detected.
//
//               This class is not necessary when the hooks are
//               detected and processed entirely by the scripting
//               language, e.g. via Scheme hooks or the messenger
//               in Python.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA EventHandler : public TypedObject {
public:
  // Define a function type suitable for receiving events.
  typedef void EventFunction(CPT_Event);
  typedef void EventCallbackFunction(CPT_Event, void *);

PUBLISHED:
  EventHandler(EventQueue *queue);

  void process_events();

  virtual void dispatch_event(const CPT_Event &event);

  void write(ostream &out) const;

  INLINE static EventHandler *get_global_event_handler(EventQueue *queue);

public:
  bool add_hook(const string &event_name, EventFunction *function);
  bool add_hook(const string &event_name, EventCallbackFunction *function,
                void *data);
  bool has_hook(const string &event_name) const;
  bool remove_hook(const string &event_name, EventFunction *function);
  bool remove_hook(const string &event_name, EventCallbackFunction *function,
                   void *data);

  void remove_all_hooks();

protected:

  typedef pset<EventFunction *> Functions;
  typedef pmap<string, Functions> Hooks;
  typedef pair<EventCallbackFunction*, void*> CallbackFunction;
  typedef pset<CallbackFunction> CallbackFunctions;
  typedef pmap<string, CallbackFunctions> CallbackHooks;

  Hooks _hooks;
  CallbackHooks _cbhooks;
  EventQueue &_queue;

  static EventHandler *_global_event_handler;
  static void make_global_event_handler(EventQueue *queue);

private:
  void write_hook(ostream &out, const Hooks::value_type &hook) const;
  void write_cbhook(ostream &out, const CallbackHooks::value_type &hook) const;


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
