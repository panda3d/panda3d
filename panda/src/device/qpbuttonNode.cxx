// Filename: qpbuttonNode.cxx
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

#include "qpbuttonNode.h"
#include "config_device.h"
#include "dataNodeTransmit.h"
#include "buttonEventList.h"

TypeHandle qpButtonNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpButtonNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpButtonNode::
qpButtonNode(ClientBase *client, const string &device_name) :
  qpDataNode(device_name)
{
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());
  _button_events = new ButtonEventList;

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

  _button = DCAST(ClientButtonDevice, device);
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
qpButtonNode::
~qpButtonNode() {
  // When the _button pointer destructs, the ClientButtonDevice
  // disconnects itself from the ClientBase, and everything that needs
  // to get turned off does.  Magic.
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonNode::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void qpButtonNode::
output(ostream &out) const {
  qpDataNode::output(out);

  if (_button != (ClientButtonDevice *)NULL) {
    out << " (";
    _button->lock();
    _button->output_buttons(out);
    _button->unlock();
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonNode::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void qpButtonNode::
write(ostream &out, int indent_level) const {
  qpDataNode::write(out, indent_level);

  if (_button != (ClientButtonDevice *)NULL) {
    _button->lock();
    _button->write_buttons(out, indent_level + 2);
    _button->unlock();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonNode::do_transmit_data
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
void qpButtonNode::
do_transmit_data(const DataNodeTransmit &, DataNodeTransmit &output) {
  if (is_valid()) {
    _button->poll();
    _button->lock();

    (*_button_events) = (*_button->get_button_events());

    _button->get_button_events()->clear();
    _button->unlock();

    output.set_data(_button_events_output, EventParameter(_button_events));
  }
}
