/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clientDialDevice.cxx
 * @author drose
 * @date 2001-01-26
 */

#include "clientDialDevice.h"

#include "indent.h"

TypeHandle ClientDialDevice::_type_handle;



/**
 * Guarantees that there is a slot in the array for the indicated index
 * number, by filling the array up to that index if necessary.
 */
void ClientDialDevice::
ensure_dial_index(int index) {
  nassertv(index >= 0);

  _dials.reserve(index + 1);
  while ((int)_dials.size() <= index) {
    _dials.push_back(DialState());
  }
}
