/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ioPtaDatagramInt.cxx
 * @author jason
 * @date 2000-06-26
 */

#include "pandabase.h"

#include "ioPtaDatagramInt.h"
#include "datagram.h"
#include "datagramIterator.h"

/**
 *
 */
void IoPtaDatagramInt::
write_datagram(BamWriter *, Datagram &dest, CPTA_int array) {
  dest.add_uint32(array.size());
  for(int i = 0; i < (int)array.size(); ++i) {
    dest.add_uint32(array[i]);
  }
}

/**
 *
 */
PTA_int IoPtaDatagramInt::
read_datagram(BamReader *, DatagramIterator &source) {
  PTA_int array;

  int size = source.get_uint32();
  for(int i = 0; i < size; ++i) {
    array.push_back(source.get_uint32());
  }

  return array;
}
