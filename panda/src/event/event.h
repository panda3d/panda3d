/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file event.h
 * @author drose
 * @date 1999-02-08
 */

// Apparently some OSX system header defines EVENT_H.  Go figure.
#ifndef __EVENT_H__
#define __EVENT_H__

#include "pandabase.h"
#include "eventParameter.h"
#include "typedReferenceCount.h"

class EventReceiver;

/**
 * A named event, possibly with parameters.  Anyone in any thread may throw an
 * event at any time; there will be one process responsible for reading and
 * dispacting on the events (but not necessarily immediately).
 *
 * This function use to inherit from Namable, but that makes it too expensive
 * to get its name the Python code.  Now it just copies the Namable interface
 * in.
 */
class EXPCL_PANDA_EVENT Event : public TypedReferenceCount {
PUBLISHED:
  Event(const std::string &event_name, EventReceiver *receiver = nullptr);
  Event(const Event &copy);
  void operator = (const Event &copy);
  ~Event();

  INLINE void set_name(const std::string &name);
  INLINE void clear_name();
  INLINE bool has_name() const;
  INLINE const std::string &get_name() const;

  void add_parameter(const EventParameter &obj);

  int get_num_parameters() const;
  EventParameter get_parameter(int n) const;
  MAKE_SEQ(get_parameters, get_num_parameters, get_parameter);

  bool has_receiver() const;
  EventReceiver *get_receiver() const;
  void set_receiver(EventReceiver *receiver);
  void clear_receiver();

  void output(std::ostream &out) const;

  MAKE_PROPERTY(name, get_name, set_name);
  MAKE_SEQ_PROPERTY(parameters, get_num_parameters, get_parameter);
  MAKE_PROPERTY2(receiver, has_receiver, get_receiver, set_receiver, clear_receiver);

protected:
  typedef pvector<EventParameter> ParameterList;
  ParameterList _parameters;
  EventReceiver *_receiver;

private:
  std::string _name;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "Event",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const Event &n);

#include "event.I"

#endif
