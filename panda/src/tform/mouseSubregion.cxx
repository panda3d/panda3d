/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseSubregion.cxx
 * @author drose
 * @date 2005-05-13
 */

#include "mouseSubregion.h"
#include "dataNodeTransmit.h"

TypeHandle MouseSubregion::_type_handle;

/**
 *
 */
MouseSubregion::
MouseSubregion(const std::string &name) :
  MouseInterfaceNode(name)
{
  _pixel_xy_input = define_input("pixel_xy", EventStoreVec2::get_class_type());
  _pixel_size_input = define_input("pixel_size", EventStoreVec2::get_class_type());
  _xy_input = define_input("xy", EventStoreVec2::get_class_type());
  _button_events_input = define_input("button_events", ButtonEventList::get_class_type());

  _pixel_xy_output = define_output("pixel_xy", EventStoreVec2::get_class_type());
  _pixel_size_output = define_output("pixel_size", EventStoreVec2::get_class_type());
  _xy_output = define_output("xy", EventStoreVec2::get_class_type());
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());

  _pixel_xy = new EventStoreVec2(LPoint2(0.0f, 0.0f));
  _pixel_size = new EventStoreVec2(LPoint2(0.0f, 0.0f));
  _xy = new EventStoreVec2(LPoint2(0.0f, 0.0f));
  _button_events = new ButtonEventList;
}

/**
 *
 */
MouseSubregion::
~MouseSubregion() {
}

/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void MouseSubregion::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &input,
                 DataNodeTransmit &output) {
  bool has_mouse = false;

  if (input.has_data(_xy_input)) {
    const EventStoreVec2 *xy;
    DCAST_INTO_V(xy, input.get_data(_xy_input).get_ptr());
    const LVecBase2 &p = xy->get_value();

    // Scale the old value into the new range.
    LVecBase2 n((p[0] - _minx) * _scalex - 1.0f, (p[1] - _miny) * _scaley - 1.0f);

    // If the mouse is indeed within the display region, pass it down.
    if (n[0] >= -1.0f && n[0] <= 1.0f &&
        n[1] >= -1.0f && n[1] <= 1.0f) {
      _xy->set_value(n);
      output.set_data(_xy_output, EventParameter(_xy));

      // Also compute the pixel coordinates, based on the supplied pixel_size.
      if (input.has_data(_pixel_size_input)) {
        const EventStoreVec2 *pixel_size;
        DCAST_INTO_V(pixel_size, input.get_data(_pixel_size_input).get_ptr());
        const LVecBase2 &s = pixel_size->get_value();

        PN_stdfloat xf = (1.0f + n[0]) * 0.5f * s[0];
        PN_stdfloat yf = (1.0f - n[1]) * 0.5f * s[1];

        _pixel_xy->set_value(LPoint2(xf, yf));
        output.set_data(_pixel_xy_output, EventParameter(_pixel_xy));
      }

      has_mouse = true;
    }
  }

  if (has_mouse) {
    // If we have the mouse, send all of the mouse buttons.
    output.set_data(_button_events_output, input.get_data(_button_events_input));
  } else {
    // Otherwise, send only the button-up events.
    _button_events->clear();

    if (input.has_data(_button_events_input)) {
      const ButtonEventList *button_events;
      DCAST_INTO_V(button_events, input.get_data(_button_events_input).get_ptr());
      int num_events = button_events->get_num_events();
      for (int i = 0; i < num_events; i++) {
        const ButtonEvent &be = button_events->get_event(i);
        if (be._type == ButtonEvent::T_up) {
          // Don't suppress this button event; pass it through.
          _button_events->add_event(be);
        }
      }
    }

    if (_button_events->get_num_events() != 0) {
      output.set_data(_button_events_output, EventParameter(_button_events));
    }
  }


  // Now scale the window size.
  if (input.has_data(_pixel_size_input)) {
    const EventStoreVec2 *pixel_size;
    DCAST_INTO_V(pixel_size, input.get_data(_pixel_size_input).get_ptr());
    const LVecBase2 &s = pixel_size->get_value();

    LVecBase2 n(s[0] * (_r - _l), s[1] * (_t - _b));
    _pixel_size->set_value(n);
    output.set_data(_pixel_size_output, EventParameter(_pixel_size));
  }
}
