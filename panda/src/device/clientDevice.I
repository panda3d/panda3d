// Filename: clientDevice.I
// Created by:  drose (25Jan01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::get_client
//       Access: Public
//  Description: Returns the ClientBase this device is associated
//               with.
////////////////////////////////////////////////////////////////////
INLINE ClientBase *ClientDevice::
get_client() const {
  return _client;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::is_connected
//       Access: Public
//  Description: Returns true if the device is still connected to its
//               ClientBase, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ClientDevice::
is_connected() const {
  return _is_connected;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::get_device_type
//       Access: Public
//  Description: Returns the type of device this is considered to be
//               to the ClientBase: a ClientTrackerDevice,
//               ClientAnalogDevice, or what have you.  This is not
//               exactly the same thing as get_type(), because it does
//               not return the exact type of the ClientDevice
//               (e.g. it reports ClientTrackerDevice, not
//               VrpnTrackerDevice).
////////////////////////////////////////////////////////////////////
INLINE TypeHandle ClientDevice::
get_device_type() const {
  return _device_type;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::get_device_name
//       Access: Public
//  Description: Returns the device name reported to the ClientBase.
//               This has some implementation-defined meaning to
//               identify particular devices.
////////////////////////////////////////////////////////////////////
INLINE const string &ClientDevice::
get_device_name() const {
  return _device_name;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::lock
//       Access: Public
//  Description: Grabs the mutex associated with this particular
//               device.  The device will not update asynchronously
//               while the mutex is held, allowing the user to copy
//               the data out without fear of getting a partial update
//               during the copy.
////////////////////////////////////////////////////////////////////
INLINE void ClientDevice::
lock() {
#ifdef OLD_HAVE_IPC
  _lock.lock();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::unlock
//       Access: Public
//  Description: Releases the mutex associated with this particular
//               device.  This should be called after all the data has
//               been successfully copied out.  See lock().
////////////////////////////////////////////////////////////////////
INLINE void ClientDevice::
unlock() {
#ifdef OLD_HAVE_IPC
  _lock.unlock();
#endif
}
