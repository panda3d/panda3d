// Filename: event.h
// Created by:  drose (08Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef EVENT_H
#define EVENT_H

#include <pandabase.h>

#include "eventParameter.h"


#include <typedReferenceCount.h>
#include <namable.h>

class EventReceiver;

////////////////////////////////////////////////////////////////////
// 	 Class : Event
// Description : A named event, possibly with parameters.  Anyone in
//               any thread may throw an event at any time; there will
//               be one process responsible for reading and dispacting
//               on the events (but not necessarily immediately).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Event : public TypedReferenceCount, public Namable {
public:
  Event(const string &event_name, EventReceiver *receiver = NULL);
  Event(const Event &copy);
  void operator = (const Event &copy);
  ~Event();

  void add_parameter(const EventParameter &obj);

  int get_num_parameters() const;
  EventParameter get_parameter(int n) const;

  bool has_receiver() const;
  EventReceiver *get_receiver() const;
  void set_receiver(EventReceiver *receiver);
  void clear_receiver();

protected:
  typedef vector<EventParameter> ParameterList;
  ParameterList _parameters;
  EventReceiver *_receiver;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "Event",
		  TypedReferenceCount::get_class_type(),
		  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
