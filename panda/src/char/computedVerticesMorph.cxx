// Filename: computedVerticesMorph.cxx
// Created by:  jason (23Jun00)
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


#include "computedVerticesMorph.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue2::write_datagram
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVerticesMorphValue2::
write_datagram(Datagram &dest)
{
  dest.add_uint16(_index);
  _vector.write_datagram(dest);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue2::read_datagram
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVerticesMorphValue2::
read_datagram(DatagramIterator &source)
{
  _index = source.get_uint16();
  _vector.read_datagram(source);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue3::write_datagram
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVerticesMorphValue3::
write_datagram(Datagram &dest)
{
  dest.add_uint16(_index);
  _vector.write_datagram(dest);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue3::read_datagram
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVerticesMorphValue3::
read_datagram(DatagramIterator &source)
{
  _index = source.get_uint16();
  _vector.read_datagram(source);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue4::write_datagram
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVerticesMorphValue4::
write_datagram(Datagram &dest)
{
  dest.add_uint16(_index);
  _vector.write_datagram(dest);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue4::read_datagram
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVerticesMorphValue4::
read_datagram(DatagramIterator &source)
{
  _index = source.get_uint16();
  _vector.read_datagram(source);
}










