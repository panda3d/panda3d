/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerData.cxx
 * @author drose
 * @date 1999-02-08
 */

#include "pointerData.h"

PointerData::
PointerData(int id, bool primary, PointerType type) :
  _id(id),
  _primary(primary),
  _type(type)
{

}

PointerData PointerData::
make_primary_mouse() {
  return PointerData(0, true, PointerType::mouse);
}

/**
 *
 */
void PointerData::
output(std::ostream &out) const {
  out << "PointerData: (" << _xpos << ", " << _ypos << ")";
}
