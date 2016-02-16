// Filename: buttonNode.cxx
// Created by:  drose (12Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "buttonNode.h"
#include "config_device.h"
#include "dataNodeTransmit.h"
#include "buttonEventList.h"
#include "dcast.h"

TypeHandle ButtonNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ButtonNode::
ButtonNode(ClientBase *client, const string &device_name) :
  DataNode(device_name)
{
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());

  nassertv(client != (ClientBase *)NULL);
  PT(ClientDevice) device =
    client->get_device(ClientButtonDevice::get_class_type(), device_name);

  if (device == (ClientDevice *)NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ButtonNode::
ButtonNode(InputDevice *device) :
  DataNode(device->get_name()),
  _device(device)
{
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());
  _device = device;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ButtonNode::
~ButtonNode() {
  // When the _device pointer destructs, the ClientButtonDevice
  // disconnects itself from the ClientBase, and everything that needs
  // to get turned off does.  Magic.
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonNode::
output(ostream &out) const {
  DataNode::output(out);

  if (_device != (InputDevice *)NULL) {
    out << " (";
    _device->output_buttons(out);
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonNode::
write(ostream &out, int indent_level) const {
  DataNode::write(out, indent_level);

  if (_device != (InputDevice *)NULL) {
    _device->write_buttons(out, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::do_transmit_data
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
void ButtonNode::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &, 
                 DataNodeTransmit &output) {
  if (is_valid()) {
    PT(ButtonEventList) bel = _device->get_button_events();
    output.set_data(_button_events_output, EventParameter(bel));
  }
}
