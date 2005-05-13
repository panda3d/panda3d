// Filename: mouseSubregion.cxx
// Created by:  drose (13May05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "mouseSubregion.h"
#include "dataNodeTransmit.h"

TypeHandle MouseSubregion::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MouseSubregion::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MouseSubregion::
MouseSubregion(const string &name) :
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

  _pixel_xy = new EventStoreVec2(LPoint2f(0.0f, 0.0f));
  _xy = new EventStoreVec2(LPoint2f(0.0f, 0.0f));
  _pixel_size = new EventStoreVec2(LPoint2f(0.0f, 0.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: MouseSubregion::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
MouseSubregion::
~MouseSubregion() {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseSubregion::do_transmit_data
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
void MouseSubregion::
do_transmit_data(const DataNodeTransmit &input, DataNodeTransmit &output) {
  if (input.has_data(_xy_input)) {
    const EventStoreVec2 *xy;
    DCAST_INTO_V(xy, input.get_data(_xy_input).get_ptr());
    const LVecBase2f &p = xy->get_value();

    // Scale the old value into the new range.
    LVecBase2f n((p[0] - _minx) * _scalex - 1.0f, (p[1] - _miny) * _scaley - 1.0f);

    // If the mouse is indeed within the display region, pass it down.
    if (n[0] >= -1.0f && n[0] <= 1.0f &&
        n[1] >= -1.0f && n[1] <= 1.0f) {
      _xy->set_value(n);
      output.set_data(_xy_output, EventParameter(_xy));

      // Also compute the pixel coordinates, based on the supplied
      // pixel_size.
      if (input.has_data(_pixel_size_input)) {
        const EventStoreVec2 *pixel_size;
        DCAST_INTO_V(pixel_size, input.get_data(_pixel_size_input).get_ptr());
        const LVecBase2f &s = pixel_size->get_value();

        float xf = (1.0f + n[0]) * 0.5f * s[0];
        float yf = (1.0f - n[1]) * 0.5f * s[1];
        
        _pixel_xy->set_value(LPoint2f(xf, yf));
        output.set_data(_pixel_xy_output, EventParameter(_pixel_xy));
      }

      // Only send the button events if the mouse is within the
      // display region.
      output.set_data(_button_events_output, input.get_data(_button_events_input));
    }
  }

  // Now scale the window size.
  if (input.has_data(_pixel_size_input)) {
    const EventStoreVec2 *pixel_size;
    DCAST_INTO_V(pixel_size, input.get_data(_pixel_size_input).get_ptr());
    const LVecBase2f &s = pixel_size->get_value();

    LVecBase2f n(s[0] * (_r - _l), s[1] * (_t - _b));
    _pixel_size->set_value(n);
    output.set_data(_pixel_size_output, EventParameter(_pixel_size));
  }
}
