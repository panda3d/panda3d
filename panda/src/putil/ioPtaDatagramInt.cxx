// Filename: ioPtaDatagramInt.cxx
// Created by:  jason (26Jun00)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

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

