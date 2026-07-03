/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlEvent.h
 * @author tkfoss
 * @date 2026-06-18
 */

#ifndef RML_EVENT_H
#define RML_EVENT_H

#include "config_rmlui.h"
#include "callbackData.h"
#include "pointerTo.h"

class RmlElement;

#ifndef CPPPARSER
namespace Rml {
  class Event;
}
#endif

/**
 * The CallbackData passed to a callback registered with
 * RmlElement::add_event_listener().  Wraps the underlying Rml::Event.
 *
 * The wrapped event is only valid for the duration of the callback; do not
 * store an RmlEvent (or anything returned by its element accessors) past the
 * return of the callback.
 */
class EXPCL_PANDARMLUI RmlEvent : public CallbackData {
PUBLISHED:
  // The DOM event type, e.g. "click", "mousemove", "submit".
  // (Named get_event_type to avoid clashing with TypedObject::get_type.)
  std::string get_event_type() const;

  // Looks up a named event parameter (mouse_x, button, value, …).  Returns
  // default_value if the parameter is absent.
  std::string get_parameter(const std::string &name,
                            const std::string &default_value = std::string()) const;
  double get_parameter_float(const std::string &name,
                             double default_value = 0.0) const;
  int get_parameter_int(const std::string &name,
                        int default_value = 0) const;
  bool has_parameter(const std::string &name) const;

  // Convenience accessors for the common mouse parameters.
  float get_mouse_x() const;
  float get_mouse_y() const;

  MAKE_PROPERTY(event_type, get_event_type);
  MAKE_PROPERTY(mouse_x, get_mouse_x);
  MAKE_PROPERTY(mouse_y, get_mouse_y);

  // The element the event was dispatched to / is currently bubbling through.
  // The returned wrapper is non-owning and valid only during the callback.
  PT(RmlElement) get_target_element() const;
  PT(RmlElement) get_current_element() const;

  // Stops the event from propagating further up the DOM tree.
  void stop_propagation();

  virtual void output(std::ostream &out) const;

public:
  RmlEvent() = default;
#ifndef CPPPARSER
  explicit RmlEvent(Rml::Event *event) : _event(event) {}

private:
  Rml::Event *_event = nullptr;
#endif

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    register_type(_type_handle, "RmlEvent",
                  CallbackData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() { init_type(); return get_class_type(); }

private:
  static TypeHandle _type_handle;
};

#endif
