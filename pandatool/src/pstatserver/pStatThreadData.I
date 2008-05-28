// Filename: pStatThreadData.I
// Created by:  drose (10Jul00)
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
//     Function: PStatThreadData::get_client_data
//       Access: Public
//  Description: Returns a pointer to the ClientData structure
//               associated with this data.
////////////////////////////////////////////////////////////////////
INLINE const PStatClientData *PStatThreadData::
get_client_data() const {
  return _client_data;
}
