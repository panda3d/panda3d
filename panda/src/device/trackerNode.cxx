/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file trackerNode.cxx
 * @author drose
 * @date 2002-03-12
 */

#include "trackerNode.h"

#include "config_device.h"
#include "clientTrackerDevice.h"
#include "dataNodeTransmit.h"

TypeHandle TrackerNode::_type_handle;

/**
 *
 */
TrackerNode::
TrackerNode(ClientBase *client, const std::string &device_name) :
  DataNode(device_name)
{
  _transform_output = define_output("transform", TransformState::get_class_type());

  _transform = TransformState::make_identity();

  nassertv(client != nullptr);
  set_tracker_coordinate_system(client->get_coordinate_system());
  set_graph_coordinate_system(CS_default);

  PT(ClientDevice) device =
    client->get_device(ClientTrackerDevice::get_class_type(), device_name);

  if (device == nullptr) {
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

  _tracker = device;
}

/**
 *
 */
TrackerNode::
TrackerNode(InputDevice *device) :
  DataNode(device->get_name()),
  _tracker(device)
{
  _transform_output = define_output("transform", TransformState::get_class_type());

  _transform = TransformState::make_identity();

  nassertv(device != nullptr);
  nassertv(device->has_tracker());

  //TODO: get coordinate system from tracker object?
  set_tracker_coordinate_system(CS_default);
  set_graph_coordinate_system(CS_default);
}

/**
 *
 */
TrackerNode::
~TrackerNode() {
  // When the _tracker pointer destructs, the ClientTrackerDevice disconnects
  // itself from the ClientBase, and everything that needs to get turned off
  // does.  Magic.
}

/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void TrackerNode::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &,
                 DataNodeTransmit &output) {
  if (is_valid()) {
    _tracker->poll();
    _data = _tracker->get_tracker();

    _data.get_orient().extract_to_matrix(_mat);
    if (_tracker_cs != _graph_cs) {
      // Convert the rotation for passing down the data graph.
      _mat = _mat * LMatrix4::convert_mat(_tracker_cs, _graph_cs);
    }
    _mat.set_row(3, _data.get_pos());

    // Now send our matrix down the pipe.  TODO: store this componentwise
    // instead of just as a matrix-based transform.
    _transform = TransformState::make_mat(_mat);
    output.set_data(_transform_output, EventParameter(_transform));
  }
}
