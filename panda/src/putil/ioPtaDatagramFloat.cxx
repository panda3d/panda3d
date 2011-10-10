// Filename: ioPtaDatagramFloat.cxx
// Created by:  charles (10Jul00)
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

#include "pandabase.h"

#include "ioPtaDatagramFloat.h"
#include "datagram.h"
#include "datagramIterator.h"

////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagramFloat::write_datagram
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void IoPtaDatagramFloat::
write_datagram(BamWriter *, Datagram &dest, CPTA_stdfloat array) {
  dest.add_uint32(array.size());
  for (int i = 0; i < (int)array.size(); ++i) {
    dest.add_stdfloat(array[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagramFloat::read_datagram
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
PTA_stdfloat IoPtaDatagramFloat::
read_datagram(BamReader *, DatagramIterator &source) {
  PTA_stdfloat array;

  int size = source.get_uint32();
  for (int i = 0; i < size; ++i) {
    array.push_back(source.get_stdfloat());
  }

  return array;
}
