// Filename: ioPtaDatagramShort.cxx
// Created by:  jason (26Jun00)
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

#include "ioPtaDatagramShort.h"
#include "datagram.h"
#include "datagramIterator.h"

////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagramShort::write_datagram
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void IoPtaDatagramShort::
write_datagram(Datagram &dest, CPTA_ushort array)
{
  dest.add_uint32(array.size());
  for(int i = 0; i < (int)array.size(); i++)
  {
    dest.add_uint16(array[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagramShort::read_datagram
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
PTA_ushort IoPtaDatagramShort::
read_datagram(DatagramIterator &source)
{
  PTA_ushort array;

  int size = source.get_uint32();
  for(int i = 0; i < size; i++)
  {
    array.push_back(source.get_uint16());
  }

  return array;
}
