/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseAndKeyboard.cxx
 * @author drose
 * @date 2002-03-12
 */

#include "mouseAndKeyboard.h"
#include "mouseData.h"
#include "buttonHandle.h"
#include "buttonEvent.h"
#include "dataNodeTransmit.h"
#include "graphicsWindow.h"

TypeHandle MouseAndKeyboard::_type_handle;

/**
 *
 */
MouseAndKeyboard::
MouseAndKeyboard(GraphicsWindow *window, int device, const std::string &name) :
  DataNode(name),
  _window(window),
  _device(window->get_input_device(device))
{
  _pixel_xy_output = define_output("pixel_xy", EventStoreVec2::get_class_type());
  _pixel_size_output = define_output("pixel_size", EventStoreVec2::get_class_type());
  _xy_output = define_output("xy", EventStoreVec2::get_class_type());
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());
  _pointer_events_output = define_output("pointer_events", PointerEventList::get_class_type());

  _pixel_xy = new EventStoreVec2(LPoint2(0.0f, 0.0f));
  _pixel_size = new EventStoreVec2(LPoint2(0.0f, 0.0f));
  _xy = new EventStoreVec2(LPoint2(0.0f, 0.0f));

  _device->enable_pointer_events();
}

/**
 * Redirects the class to get the data from the mouse and keyboard associated
 * with a different window and/or device number.
 */
void MouseAndKeyboard::
set_source(GraphicsWindow *window, int device) {
  _window = window;
  _device = window->get_input_device(device);

  //_device->enable_pointer_events();
}

/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void MouseAndKeyboard::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &,
                 DataNodeTransmit &output) {

  GraphicsWindowInputDevice *device = (GraphicsWindowInputDevice *)_device.p();

  if (device->has_button_event()) {
    PT(ButtonEventList) bel = device->get_button_events();
    output.set_data(_button_events_output, EventParameter(bel));
  }
  if (device->has_pointer_event()) {
    PT(PointerEventList) pel = device->get_pointer_events();
    output.set_data(_pointer_events_output, EventParameter(pel));
  }

  // Get the window size.
  WindowProperties properties = _window->get_properties();
  if (properties.has_size()) {
    int w = properties.get_x_size();
    int h = properties.get_y_size();

    _pixel_size->set_value(LPoint2(w, h));
    output.set_data(_pixel_size_output, EventParameter(_pixel_size));

    if (device->has_pointer()) {
      PointerData mdata = device->get_pointer();

      if (mdata._in_window) {
        // Get mouse motion in pixels.
        _pixel_xy->set_value(LPoint2(mdata._xpos, mdata._ypos));
        output.set_data(_pixel_xy_output, EventParameter(_pixel_xy));

        // Normalize pixel motion to range [-1,1].
        PN_stdfloat xf = (PN_stdfloat)(2 * mdata._xpos) / (PN_stdfloat)w - 1.0f;
        PN_stdfloat yf = 1.0f - (PN_stdfloat)(2 * mdata._ypos) / (PN_stdfloat)h;

        _xy->set_value(LPoint2(xf, yf));
        output.set_data(_xy_output, EventParameter(_xy));
      }
    }
  }
}
