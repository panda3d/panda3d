// Filename: clientDialDevice.cxx
// Created by:  drose (26Jan01)
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

#include "device_headers.h"
#pragma hdrstop

TypeHandle ClientDialDevice::_type_handle;



////////////////////////////////////////////////////////////////////
//     Function: ClientDialDevice::ensure_dial_index
//       Access: Private
//  Description: Guarantees that there is a slot in the array for the
//               indicated index number, by filling the array up to
//               that index if necessary.
////////////////////////////////////////////////////////////////////
void ClientDialDevice::
ensure_dial_index(int index) {
  nassertv(index >= 0);

  _dials.reserve(index + 1);
  while ((int)_dials.size() <= index) {
    _dials.push_back(DialState());
  }
}
