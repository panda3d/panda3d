/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clientAnalogDevice.I
 * @author drose
 * @date 2001-01-26
 */

/**
 *
 */
INLINE ClientAnalogDevice::
ClientAnalogDevice(ClientBase *client, const std::string &device_name):
  ClientDevice(client, get_class_type(), device_name)
{
}
