// Filename: mouseAndKeyboard.cxx
// Created by:  drose (12Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "mouseAndKeyboard.h"
#include "mouseData.h"
#include "buttonHandle.h"
#include "buttonEvent.h"
#include "dataNodeTransmit.h"

TypeHandle qpMouseAndKeyboard::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpMouseAndKeyboard::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpMouseAndKeyboard::
qpMouseAndKeyboard(GraphicsWindow *window, int device, const string &name) :
  qpDataNode(name),
  _window(window),
  _device(device)
{
  _pixel_xy_output = define_output("pixel_xy", EventStoreVec2::get_class_type());
  _xy_output = define_output("xy", EventStoreVec2::get_class_type());
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());

  _pixel_xy = new EventStoreVec2(LPoint2f(0.0f, 0.0f));
  _xy = new EventStoreVec2(LPoint2f(0.0f, 0.0f));
  _button_events = new ButtonEventList;
}

////////////////////////////////////////////////////////////////////
//     Function: qpMouseAndKeyboard::do_transmit_data
//       Access: Protected, Virtual
//  Description: The virtual implementation of transmit_data().  This
//               function receives an array of input parameters and
//               should generate an array of output parameters.  The
//               input parameters may be accessed with the index
//               numbers returned by the define_input() calls that
//               were made earlier (presumably in the constructor);
//               likewise, the output parameters should be set with
//               the index numbers returned by the define_output()
//               calls.
////////////////////////////////////////////////////////////////////
void qpMouseAndKeyboard::
do_transmit_data(const DataNodeTransmit &, DataNodeTransmit &output) {
  if (_window->has_button_event(_device)) {
    // Fill up the button events.
    _button_events->clear();
    while (_window->has_button_event(_device)) {
      ButtonEvent be = _window->get_button_event(_device);
      _button_events->add_event(be);
    }
    output.set_data(_button_events_output, EventParameter(_button_events));
  }

  if (_window->has_pointer(_device)) {
    const MouseData &mdata = _window->get_mouse_data(_device);

    if (mdata._in_window) {
      // Get mouse motion in pixels.
      _pixel_xy->set_value(LPoint2f(mdata._xpos, mdata._ypos));
      output.set_data(_pixel_xy_output, EventParameter(_pixel_xy));

      int w = _window->get_width();
      int h = _window->get_height();

      // Normalize pixel motion to range [-1,1].
      float xf = (float)(2 * mdata._xpos) / (float)w - 1.0f;
      float yf = 1.0f - (float)(2 * mdata._ypos) / (float)h;

      _xy->set_value(LPoint2f(xf, yf));
      output.set_data(_xy_output, EventParameter(_xy));
    }
  }
}
