// Filename: dataNodeTransmit.I
// Created by:  drose (11Mar02)
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
//     Function: DataNodeTransmit::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DataNodeTransmit::
DataNodeTransmit() {
}

////////////////////////////////////////////////////////////////////
//     Function: DataNodeTransmit::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DataNodeTransmit::
DataNodeTransmit(const DataNodeTransmit &copy) :
  _data(copy._data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DataNodeTransmit::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DataNodeTransmit::
operator = (const DataNodeTransmit &copy) {
  _data = copy._data;
}

////////////////////////////////////////////////////////////////////
//     Function: DataNodeTransmit::reserve
//       Access: Public
//  Description: Tells the DataNodeTransmit object how many wires it
//               is expected to store data for.
////////////////////////////////////////////////////////////////////
INLINE void DataNodeTransmit::
reserve(int num_wires) {
  _data.reserve(num_wires);
}

////////////////////////////////////////////////////////////////////
//     Function: DataNodeTransmit::get_data
//       Access: Public
//  Description: Extracts the data for the indicated index, if it has
//               been stored, or the empty parameter if it has not.
////////////////////////////////////////////////////////////////////
INLINE const EventParameter &DataNodeTransmit::
get_data(int index) const {
  if (index >= 0 && index < (int)_data.size()) {
    return _data[index];
  }
  static EventParameter empty_parameter;
  return empty_parameter;
}

////////////////////////////////////////////////////////////////////
//     Function: DataNodeTransmit::has_data
//       Access: Public
//  Description: Returns true if the indicated parameter has been
//               stored, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DataNodeTransmit::
has_data(int index) const {
  if (index >= 0 && index < (int)_data.size()) {
    return !_data[index].is_empty();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DataNodeTransmit::set_data
//       Access: Public
//  Description: Sets the data for the indicated parameter.
////////////////////////////////////////////////////////////////////
INLINE void DataNodeTransmit::
set_data(int index, const EventParameter &data) {
  if (index >= (int)_data.size()) {
    slot_data(index);
  }
  nassertv(index >= 0 && index < (int)_data.size());
  _data[index] = data;
}
