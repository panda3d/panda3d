// Filename: qpanalogNode.cxx
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

#include "qpanalogNode.h"
#include "config_device.h"
#include "dataNodeTransmit.h"


TypeHandle qpAnalogNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpAnalogNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpAnalogNode::
qpAnalogNode(ClientBase *client, const string &device_name) :
  qpDataNode(device_name)
{
  _xy_output = define_output("xy", EventStoreVec2::get_class_type());
  _xy = new EventStoreVec2(LPoint2f(0.0f, 0.0f));

  nassertv(client != (ClientBase *)NULL);
  PT(ClientDevice) device =
    client->get_device(ClientAnalogDevice::get_class_type(), device_name);

  if (device == (ClientDevice *)NULL) {
    device_cat.warning()
      << "Unable to open analog device " << device_name << "\n";
    return;
  }

  if (!device->is_of_type(ClientAnalogDevice::get_class_type())) {
    device_cat.error()
      << "Inappropriate device type " << device->get_type()
      << " created; expected a ClientAnalogDevice.\n";
    return;
  }

  _analog = DCAST(ClientAnalogDevice, device);
}

////////////////////////////////////////////////////////////////////
//     Function: qpAnalogNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
qpAnalogNode::
~qpAnalogNode() {
  // When the _analog pointer destructs, the ClientAnalogDevice
  // disconnects itself from the ClientBase, and everything that needs
  // to get turned off does.  Magic.
}

////////////////////////////////////////////////////////////////////
//     Function: qpAnalogNode::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void qpAnalogNode::
write(ostream &out, int indent_level) const {
  qpDataNode::write(out, indent_level);

  if (_analog != (ClientAnalogDevice *)NULL) {
    _analog->lock();
    _analog->write_controls(out, indent_level + 2);
    _analog->unlock();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpAnalogNode::do_transmit_data
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
void qpAnalogNode::
do_transmit_data(const DataNodeTransmit &, DataNodeTransmit &output) {
  if (is_valid()) {
    _analog->poll();

    LPoint2f out(0.0f, 0.0f);

    _analog->lock();
    for (int i = 0; i < max_outputs; i++) {
      if (_outputs[i]._index >= 0 &&
          _analog->is_control_known(_outputs[i]._index)) {
        if (_outputs[i]._flip) {
          out[i] = -_analog->get_control_state(_outputs[i]._index);
        } else {
          out[i] = _analog->get_control_state(_outputs[i]._index);
        }
      }
    }
    _analog->unlock();
    _xy->set_value(out);
    output.set_data(_xy_output, EventParameter(_xy));
  }
}
