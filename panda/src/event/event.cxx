/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file event.cxx
 * @author drose
 * @date 1999-02-08
 */

#include "event.h"
#include "config_event.h"

TypeHandle Event::_type_handle;

/**
 *
 */
Event::
Event(const std::string &event_name, EventReceiver *receiver) :
  _name(event_name)
{
  _receiver = receiver;
}

/**
 *
 */
Event::
Event(const Event &copy) :
  _parameters(copy._parameters),
  _receiver(copy._receiver),
  _name(copy._name)
{
}

/**
 *
 */
void Event::
operator = (const Event &copy) {
  _parameters = copy._parameters;
  _receiver = copy._receiver;
  _name = copy._name;
}

/**
 *
 */
Event::
~Event() {
}

/**
 *
 */
void Event::
add_parameter(const EventParameter &obj) {
  _parameters.push_back(obj);
}


/**
 *
 */
int Event::
get_num_parameters() const {
  return _parameters.size();
}

/**
 *
 */
EventParameter Event::
get_parameter(int n) const {
  nassertr(n >= 0 && n < (int)_parameters.size(), EventParameter(0));
  return _parameters[n];
}


/**
 *
 */
bool Event::
has_receiver() const {
  return _receiver != nullptr;
}

/**
 *
 */
EventReceiver *Event::
get_receiver() const {
  return _receiver;
}

/**
 *
 */
void Event::
set_receiver(EventReceiver *receiver) {
  _receiver = receiver;
}

/**
 *
 */
void Event::
clear_receiver() {
  _receiver = nullptr;
}

/**
 *
 */
void Event::
output(std::ostream &out) const {
  out << get_name();

  out << "(";
  for (ParameterList::const_iterator pi = _parameters.begin(); pi != _parameters.end(); ++pi) {
    if (pi != _parameters.begin()) {
      out << ", ";
    }
    out << (*pi);
  }
  out << ")";
}
