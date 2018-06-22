/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file analogNode.cxx
 * @author drose
 * @date 2002-03-12
 */

#include "analogNode.h"
#include "config_device.h"
#include "dataNodeTransmit.h"
#include "dcast.h"


TypeHandle AnalogNode::_type_handle;

/**
 *
 */
AnalogNode::
AnalogNode(ClientBase *client, const std::string &device_name) :
  DataNode(device_name)
{
  _xy_output = define_output("xy", EventStoreVec2::get_class_type());
  _xy = new EventStoreVec2(LPoint2(0.0f, 0.0f));

  nassertv(client != nullptr);
  PT(ClientDevice) device =
    client->get_device(ClientAnalogDevice::get_class_type(), device_name);

  if (device == nullptr) {
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

/**
 *
 */
AnalogNode::
~AnalogNode() {
  // When the _analog pointer destructs, the ClientAnalogDevice disconnects
  // itself from the ClientBase, and everything that needs to get turned off
  // does.  Magic.
}

/**
 *
 */
void AnalogNode::
write(std::ostream &out, int indent_level) const {
  DataNode::write(out, indent_level);

  if (_analog != nullptr) {
    _analog->acquire();
    _analog->write_controls(out, indent_level + 2);
    _analog->unlock();
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
void AnalogNode::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &,
                 DataNodeTransmit &output) {
  if (is_valid()) {
    _analog->poll();

    LPoint2 out(0.0f, 0.0f);

    _analog->acquire();
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
