// Filename: qptrackerNode.cxx
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

#include "qptrackerNode.h"
#include "config_device.h"

TypeHandle qpTrackerNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpTrackerNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpTrackerNode::
qpTrackerNode(ClientBase *client, const string &device_name) :
  qpDataNode(device_name)
{
  _transform_output = define_output("transform", EventStoreMat4::get_class_type());

  _transform = new EventStoreMat4(LMatrix4f::ident_mat());

  nassertv(client != (ClientBase *)NULL);
  set_tracker_coordinate_system(client->get_coordinate_system());
  set_graph_coordinate_system(CS_default);

  PT(ClientDevice) device =
    client->get_device(ClientTrackerDevice::get_class_type(), device_name);

  if (device == (ClientDevice *)NULL) {
    device_cat.warning()
      << "Unable to open tracker device " << device_name << "\n";
    return;
  }

  if (!device->is_of_type(ClientTrackerDevice::get_class_type())) {
    device_cat.error()
      << "Inappropriate device type " << device->get_type()
      << " created; expected a ClientTrackerDevice.\n";
    return;
  }

  _tracker = DCAST(ClientTrackerDevice, device);
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackerNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
qpTrackerNode::
~qpTrackerNode() {
  // When the _tracker pointer destructs, the ClientTrackerDevice
  // disconnects itself from the ClientBase, and everything that needs
  // to get turned off does.  Magic.
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackerNode::do_transmit_data
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
void qpTrackerNode::
do_transmit_data(const DataNodeTransmit &, DataNodeTransmit &output) {
  if (is_valid()) {
    _tracker->poll();
    _tracker->lock();
    _data = _tracker->get_data();
    _tracker->unlock();

    _data.get_orient().extract_to_matrix(_mat);
    if (_tracker_cs != _graph_cs) {
      // Convert the rotation for passing down the data graph.
      _mat = _mat * LMatrix4f::convert_mat(_tracker_cs, _graph_cs);
    }
    _mat.set_row(3, _data.get_pos());

    // Now send our matrix down the pipe.
    _transform->set_value(_mat);
    output.set_data(_transform_output, EventParameter(_transform));
  }
}
