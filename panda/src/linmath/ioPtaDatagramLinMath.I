// Filename: ioPtaDatagramLinMath.I
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


////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagamLinMath::write_datagram
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
template<class LinMathElement>
void IoPtaDatagramLinMath<LinMathElement>::
write_datagram(Datagram &dest, CPTA(LinMathElement) array)
{
  dest.add_uint32(array.size());
  for(int i = 0; i < (int)array.size(); i++)
  {
    array[i].write_datagram(dest);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IoPtaDatagamLinMath::read_datagram
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
template<class LinMathElement>
PTA(LinMathElement) IoPtaDatagramLinMath<LinMathElement>::
read_datagram(DatagramIterator &source)
{
  PTA(LinMathElement) array;
  LinMathElement temp;

  int size = source.get_uint32();
  for(int i = 0; i < size; i++)
  {
    temp.read_datagram(source);
    array.push_back(temp);
  }

  return array;
}


