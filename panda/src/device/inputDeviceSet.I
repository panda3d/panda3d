// Filename: inputDeviceSet.I
// Created by:  rdb (16Dec15)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InputDeviceSet::
~InputDeviceSet() {
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::operator []
//       Access: Published
//  Description: Returns the nth InputDevice in the collection.
////////////////////////////////////////////////////////////////////
INLINE InputDevice *InputDeviceSet::
operator [] (size_t index) const {
  nassertr(index < _devices.size(), NULL);
  return _devices[index];
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::size
//       Access: Published
//  Description: Returns the number of devices in the collection.
////////////////////////////////////////////////////////////////////
INLINE size_t InputDeviceSet::
size() const {
  return _devices.size();
}
