// Filename: buttonNode.cxx
// Created by:  drose (31Dec69)
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


#include "buttonNode.h"
#include "config_device.h"

#include <buttonEventDataTransition.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle ButtonNode::_type_handle;

TypeHandle ButtonNode::_button_events_type;

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ButtonNode::
ButtonNode(ClientBase *client, const string &device_name) :
  DataNode(device_name)
{
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

  if (_button != (ClientButtonDevice *)NULL) {
    _button_events = new ButtonEventDataAttribute();
    _attrib.set_attribute(_button_events_type, _button_events);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ButtonNode::
~ButtonNode() {
  // When the _button pointer destructs, the ClientButtonDevice
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

  if (_button != (ClientButtonDevice *)NULL) {
    out << " (";
    _button->lock();
    _button->output_buttons(out);
    _button->unlock();
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

  if (_button != (ClientButtonDevice *)NULL) {
    _button->lock();
    _button->write_buttons(out, indent_level + 2);
    _button->unlock();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::transmit_data
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonNode::
transmit_data(NodeAttributes &data) {
  if (is_valid()) {
    _button->poll();
    _button->lock();
    (*_button_events) = (*_button->get_button_events());
    _button->get_button_events()->clear();
    _button->unlock();

    if (device_cat.is_debug()) {
      device_cat.debug() << "ButtonNode:attributes" << endl;
      _attrib.write(device_cat.debug(false), 3);
    }
  }

  data = _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonNode::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "ButtonNode",
                DataNode::get_class_type());

  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
                           ButtonEventDataTransition::get_class_type());
}

