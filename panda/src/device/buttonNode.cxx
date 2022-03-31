/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonNode.cxx
 * @author drose
 * @date 2002-03-12
 */

#include "buttonNode.h"
#include "config_device.h"
#include "dataNodeTransmit.h"
#include "buttonEventList.h"
#include "dcast.h"

TypeHandle ButtonNode::_type_handle;

/**
 *
 */
ButtonNode::
ButtonNode(ClientBase *client, const std::string &device_name) :
  DataNode(device_name)
{
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());

  nassertv(client != nullptr);
  PT(ClientDevice) device =
    client->get_device(ClientButtonDevice::get_class_type(), device_name);

  if (device == nullptr) {
    device_cat.warning()
      << "Unable to open button device " << device_name << "\n";
    return;
  }

  if (!device->is_of_type(ClientButtonDevice::get_class_type())) {
    device_cat.error()
      << "Inappropriate device type " << device->get_type()
      << " created; expected a ClientButtonDevice.\n";
    return;
  }

  _device = device;
}

/**
 *
 */
ButtonNode::
ButtonNode(InputDevice *device) :
  DataNode(device->get_name()),
  _device(device)
{
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());
  _device = device;
}

/**
 *
 */
ButtonNode::
~ButtonNode() {
  // When the _button pointer destructs, the ClientButtonDevice disconnects
  // itself from the ClientBase, and everything that needs to get turned off
  // does.  Magic.
}

/**
 *
 */
void ButtonNode::
output(std::ostream &out) const {
  DataNode::output(out);

  if (_device != nullptr) {
    out << " (";
    _device->output_buttons(out);
    out << ")";
  }
}

/**
 *
 */
void ButtonNode::
write(std::ostream &out, int indent_level) const {
  DataNode::write(out, indent_level);

  if (_device != nullptr) {
    _device->write_buttons(out, indent_level + 2);
  }
}

/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void ButtonNode::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &,
                 DataNodeTransmit &output) {
  if (is_valid()) {
    PT(ButtonEventList) bel = _device->get_button_events();
    output.set_data(_button_events_output, EventParameter(bel));
  }
}
