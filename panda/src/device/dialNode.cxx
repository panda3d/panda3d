// Filename: dialNode.cxx
// Created by:  drose (26Jan01)
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

#include "device_headers.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle DialNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DialNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DialNode::
DialNode(ClientBase *client, const string &device_name) :
  DataNode(device_name)
{
  nassertv(client != (ClientBase *)NULL);
  PT(ClientDevice) device =
    client->get_device(ClientDialDevice::get_class_type(), device_name);

  if (device == (ClientDevice *)NULL) {
    device_cat.warning()
      << "Unable to open dial device " << device_name << "\n";
    return;
  }

  if (!device->is_of_type(ClientDialDevice::get_class_type())) {
    device_cat.error()
      << "Inappropriate device type " << device->get_type()
      << " created; expected a ClientDialDevice.\n";
    return;
  }

  _dial = DCAST(ClientDialDevice, device);
}

////////////////////////////////////////////////////////////////////
//     Function: DialNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DialNode::
~DialNode() {
  // When the _dial pointer destructs, the ClientDialDevice
  // disconnects itself from the ClientBase, and everything that needs
  // to get turned off does.  Magic.
}

////////////////////////////////////////////////////////////////////
//     Function: DialNode::transmit_data
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DialNode::
transmit_data(NodeAttributes &data) {
  if (is_valid()) {
    _dial->poll();

    // Not clear yet what we should be transmitting.
  }

  data = _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: DialNode::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DialNode::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "DialNode",
                DataNode::get_class_type());
}

