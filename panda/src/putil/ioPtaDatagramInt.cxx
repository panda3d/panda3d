// Filename: ioPtaDatagramInt.cxx
// Created by:  jason (26Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "ioPtaDatagramInt.h"
#include "datagram.h"
#include "datagramIterator.h"

////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagramInt::write_datagram
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void IoPtaDatagramInt::
write_datagram(Datagram &dest, CPTA_int array)
{
  dest.add_uint32(array.size());
  for(int i = 0; i < (int)array.size(); i++)
  {
    dest.add_uint32(array[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagramInt::read_datagram
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
PTA_int IoPtaDatagramInt::
read_datagram(DatagramIterator &source)
{
  PTA_int array;

  int size = source.get_uint32();
  for(int i = 0; i < size; i++)
  {
    array.push_back(source.get_uint32());
  }

  return array;
}

