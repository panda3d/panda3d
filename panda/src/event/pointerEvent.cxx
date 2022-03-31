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
#include "datagram.h"
#include "datagramIterator.h"

/**
 *
 */
void PointerEvent::
output(std::ostream &out) const {
  out << (_in_window ? "In@" : "Out@")
      << _xpos << "," << _ypos << " ";
}

/**
 * Writes the event into a datagram.
 */
void PointerEvent::
write_datagram(Datagram &dg) const {
  nassert_raise("This function not implemented yet.");
}

/**
 * Restores the event from the datagram.
 */
void PointerEvent::
read_datagram(DatagramIterator &scan) {
  nassert_raise("This function not implemented yet.");
}
