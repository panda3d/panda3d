// Filename: clientDialDevice.cxx
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

#include "clientDialDevice.h"

#include <indent.h>

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
