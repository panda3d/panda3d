// Filename: computedVerticesMorph.cxx
// Created by:  jason (23Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "computedVerticesMorph.h"

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

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










