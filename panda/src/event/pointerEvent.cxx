/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerEvent.cxx
 * @author drose
 * @date 2000-03-01
 */

#include "pointerEvent.h"

PointerEvent::
PointerEvent(PointerData data, double time) :
  _data(data),
  _time(time)
{

}

/**
 *
 */
void PointerEvent::
output(std::ostream &out) const {
  out << "In@" << _data.get_x() << "," << _data.get_y() << " ";
}