/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file InputDeviceNode.cxx
 * @author fireclaw
 * @date 2016-07-14
 */

#include "config_device.h"
#include "inputDeviceNode.h"
#include "dataNodeTransmit.h"
#include "inputDeviceManager.h"

TypeHandle InputDeviceNode::_type_handle;

InputDeviceNode::
InputDeviceNode(InputDevice *device, const string &name) :
  DataNode(name),
  _device(device)
{
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());
  _low_battery_event_output = define_output("low_battery_level_event", EventStoreInt::get_class_type());
}

/**
 * Redirects the class to get the data from a different device.
 */
void InputDeviceNode::
set_device(InputDevice *device) {
  _device = device;
}

/**
 * Returns the associated device.
 */
PT(InputDevice) InputDeviceNode::
get_device() const {
  return _device;
}

/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void InputDeviceNode::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &,
                 DataNodeTransmit &output) {
  // get all button events of the device and forward them to the data graph
  if (_device->has_button_event()) {
    PT(ButtonEventList) bel = _device->get_button_events();
    output.set_data(_button_events_output, EventParameter(bel));
  }

  // calculate the battery level in percent and set a warning if the level is to low
  /*if (_device->has_battery()) {
    short bl = _device->get_battery_level();
    short bl_percent = bl / (_device->get_max_battery_level() / (short)100);
    if (bl_percent <= low_battery_level) {
      output.set_data(_low_battery_event_output, EventParameter(1));
    }
  }*/
}
