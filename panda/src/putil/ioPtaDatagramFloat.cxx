// Filename: ioPtaDatagramFloat.cxx
// Created by:  charles (10Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
write_datagram(Datagram &dest, CPTA_float array)
{
  dest.add_uint32(array.size());
  for (int i = 0; i < (int)array.size(); i++) {
    dest.add_float32(array[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagramFloat::read_datagram
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
PTA_float IoPtaDatagramFloat::
read_datagram(DatagramIterator &source)
{
  PTA_float array;

  int size = source.get_uint32();
  for (int i = 0; i < size; i++) {
    array.push_back(source.get_float32());
  }

  return array;
}
