// Filename: ioPtaDatagramFloat.cxx
// Created by:  charles (10Jul00)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

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
  for (int i = 0; i < array.size(); i++) {
    dest.add_float64(array[i]);
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
    array.push_back(source.get_float64());
  }

  return array;
}
