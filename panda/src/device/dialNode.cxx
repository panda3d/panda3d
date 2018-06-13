/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dialNode.cxx
 * @author drose
 * @date 2002-03-12
 */

#include "dialNode.h"
#include "config_device.h"
#include "dataNodeTransmit.h"
#include "dcast.h"

TypeHandle DialNode::_type_handle;

/**
 *
 */
DialNode::
DialNode(ClientBase *client, const std::string &device_name) :
  DataNode(device_name)
{
  nassertv(client != nullptr);
  PT(ClientDevice) device =
    client->get_device(ClientDialDevice::get_class_type(), device_name);

  if (device == nullptr) {
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

/**
 *
 */
DialNode::
~DialNode() {
  // When the _dial pointer destructs, the ClientDialDevice disconnects itself
  // from the ClientBase, and everything that needs to get turned off does.
  // Magic.
}

/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void DialNode::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &,
                 DataNodeTransmit &output) {
  if (is_valid()) {
    _dial->poll();

    // Not clear yet what we should be transmitting.
  }
}
