// Filename: eventHandler.h
// Created by:  drose (08Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <pandabase.h>

#include "event.h"
#include "pt_Event.h"

#include <set>
#include <map>

class EventQueue;

////////////////////////////////////////////////////////////////////
// 	 Class : EventHandler
// Description : A class to monitor events from the C++ side of
//               things.  It maintains a set of "hooks", function
//               pointers assigned to event names, and calls the
//               appropriate hooks when the matching event is
//               detected.
//
//               This class is not necessary when the hooks are
//               detected and processed entirely by the scripting
//               language, e.g. via Scheme hooks.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS EventHandler : public TypedObject {
public:
  // Define a function type suitable for receiving events.
  typedef void EventFunction(CPT_Event);
  typedef void EventCallbackFunction(CPT(Event), void*);

PUBLISHED:
  EventHandler(EventQueue *queue);

  void process_events();

  virtual void dispatch_event(const CPT_Event &event);

  void write(ostream &out) const;

public:
  bool add_hook(const string &event_name, EventFunction *function);
  bool add_hook(const string &event_name, EventCallbackFunction *function,
		void*);
  bool remove_hook(const string &event_name, EventFunction *function);
  bool remove_hook(const string &event_name, EventCallbackFunction *function,
		   void*);

  void remove_all_hooks();

protected:

  typedef set<EventFunction *> Functions;
  typedef map<string, Functions> Hooks;
  typedef pair<EventCallbackFunction*, void*> CallbackFunction;
  typedef set<CallbackFunction> CallbackFunctions;
  typedef map<string, CallbackFunctions> CallbackHooks;

  Hooks _hooks;
  CallbackHooks _cbhooks;
  EventQueue &_queue;

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

#endif
