/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ioPtaDatagramFloat.cxx
 * @author charles
 * @date 2000-07-10
 */

#include "pandabase.h"

#include "ioPtaDatagramFloat.h"
#include "datagram.h"
#include "datagramIterator.h"

/**
 *
 */
void IoPtaDatagramFloat::
write_datagram(BamWriter *, Datagram &dest, CPTA_stdfloat array) {
  dest.add_uint32(array.size());
  for (int i = 0; i < (int)array.size(); ++i) {
    dest.add_stdfloat(array[i]);
  }
}

/**
 *
 */
PTA_stdfloat IoPtaDatagramFloat::
read_datagram(BamReader *, DatagramIterator &source) {
  PTA_stdfloat array;

  int size = source.get_uint32();
  for (int i = 0; i < size; ++i) {
    array.push_back(source.get_stdfloat());
  }

  return array;
}
