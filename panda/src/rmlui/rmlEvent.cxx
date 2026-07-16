/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlEvent.cxx
 * @author tkfoss
 * @date 2026-06-18
 */

#include "rmlEvent.h"
#include "rmlElement.h"

#ifndef CPPPARSER
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/Element.h>
#endif

TypeHandle RmlEvent::_type_handle;

/**
 * Returns the DOM event type, e.g. "click".
 */
std::string RmlEvent::
get_event_type() const {
  nassertr(_event != nullptr, std::string());
  return _event->GetType();
}

/**
 * Returns the named event parameter as a string, or default_value if absent.
 */
std::string RmlEvent::
get_parameter(const std::string &name, const std::string &default_value) const {
  nassertr(_event != nullptr, default_value);
  return _event->GetParameter<Rml::String>(name, default_value);
}

/**
 * Returns the named event parameter as a float, or default_value if absent.
 */
double RmlEvent::
get_parameter_float(const std::string &name, double default_value) const {
  nassertr(_event != nullptr, default_value);
  return _event->GetParameter<float>(name, (float)default_value);
}

/**
 * Returns the named event parameter as an int, or default_value if absent.
 */
int RmlEvent::
get_parameter_int(const std::string &name, int default_value) const {
  nassertr(_event != nullptr, default_value);
  return _event->GetParameter<int>(name, default_value);
}

/**
 * Returns true if the event carries the named parameter.
 */
bool RmlEvent::
has_parameter(const std::string &name) const {
  nassertr(_event != nullptr, false);
  const Rml::Dictionary &params = _event->GetParameters();
  return params.find(name) != params.end();
}

/**
 * Returns the x coordinate of the mouse in the document, in pixels.
 */
float RmlEvent::
get_mouse_x() const {
  nassertr(_event != nullptr, 0.0f);
  return _event->GetParameter<float>("mouse_x", 0.0f);
}

/**
 * Returns the y coordinate of the mouse in the document, in pixels.
 */
float RmlEvent::
get_mouse_y() const {
  nassertr(_event != nullptr, 0.0f);
  return _event->GetParameter<float>("mouse_y", 0.0f);
}

/**
 * Returns the element the event was originally dispatched to.
 */
PT(RmlElement) RmlEvent::
get_target_element() const {
  nassertr(_event != nullptr, nullptr);
  Rml::Element *el = _event->GetTargetElement();
  return el ? new RmlElement(el) : nullptr;
}

/**
 * Returns the element the event is currently bubbling through.
 */
PT(RmlElement) RmlEvent::
get_current_element() const {
  nassertr(_event != nullptr, nullptr);
  Rml::Element *el = _event->GetCurrentElement();
  return el ? new RmlElement(el) : nullptr;
}

/**
 * Stops the event from propagating further up the DOM tree.
 */
void RmlEvent::
stop_propagation() {
  nassertv(_event != nullptr);
  _event->StopPropagation();
}

/**
 *
 */
void RmlEvent::
output(std::ostream &out) const {
  out << "RmlEvent(" << (_event != nullptr ? _event->GetType() : "null") << ")";
}
