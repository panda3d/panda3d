/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file axisEvent.cxx
 * @author jb
 * @date 2023-11-24
 */

#include "axisEvent.h"
#include "datagram.h"
#include "datagramIterator.h"

/**
 *
 */
AxisEvent::
AxisEvent(int index, double value) :
  index(index),
  value(value)
{
}

/**
 *
 */
void AxisEvent::
output(std::ostream &out) const {
  out << "Axis: " << index << " Value: " << value << "\n";
}

/**
 * Writes the event into a datagram.
 */
void AxisEvent::
write_datagram(Datagram &dg) const {
  dg.add_uint16(index);
  dg.add_float64(value);
}

/**
 * Restores the event from the datagram.
 */
void AxisEvent::
read_datagram(DatagramIterator &scan) {
  index = scan.get_uint16();
  value = scan.get_float64();
}
