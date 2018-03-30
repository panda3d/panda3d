/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventMapper.h
 * @author Mitchell Stokes
 * @date 2018-3-30
 */
#ifndef EVENTMAPPER_H
#define EVENTMAPPER_H

#include "pandabase.h"
#include "pmap.h"
#include "pset.h"

class Event;

/**
 * A class to map a set of input events to output events. For example, "space"
 * to "jump".
 */
class EXPCL_PANDA_EVENT EventMapper : public TypedObject {
protected:
  typedef pset<string> EventSet;
  typedef pmap<string, EventSet> EventMap;

  EventMap _event_map;;

  static EventMapper *_global_ptr;

  EventMapper();
  static void make_global_ptr();

  static void throw_events(const Event *event, EventSet *events, string suffix);
  static void event_callback(const Event *event, void *data);
  static void event_callback_up(const Event *event, void *data);
  static void event_callback_down(const Event *event, void *data);

PUBLISHED:
  INLINE static EventMapper *get_global_ptr();
  void reload_config();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "EventMapper",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


#include "eventMapper.I"
#endif //EVENTMAPPER_H
