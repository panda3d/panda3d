// Filename: physxMemoryReadBuffer.cxx
// Created by:  enn0x (11Oct09)
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

#include "physxMemoryReadBuffer.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryReadBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PhysxMemoryReadBuffer::PhysxMemoryReadBuffer(const NxU8 *data) : buffer(data)
{

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryReadBuffer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PhysxMemoryReadBuffer::~PhysxMemoryReadBuffer()
{

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryReadBuffer::readByte
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxU8 PhysxMemoryReadBuffer::readByte() const
{
  NxU8 b;
  memcpy(&b, buffer, sizeof(NxU8));
  buffer += sizeof(NxU8);
  return b;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryReadBuffer::readWord
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxU16 PhysxMemoryReadBuffer::readWord() const
{
  NxU16 w;
  memcpy(&w, buffer, sizeof(NxU16));
  buffer += sizeof(NxU16);
  return w;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryReadBuffer::readDword
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxU32 PhysxMemoryReadBuffer::readDword() const
{
  NxU32 d;
  memcpy(&d, buffer, sizeof(NxU32));
  buffer += sizeof(NxU32);
  return d;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryReadBuffer::readFloat
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
float PhysxMemoryReadBuffer::readFloat() const
{
  float f;
  memcpy(&f, buffer, sizeof(float));
  buffer += sizeof(float);
  return f;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryReadBuffer::readDouble
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
double PhysxMemoryReadBuffer::readDouble() const
{
  double f;
  memcpy(&f, buffer, sizeof(double));
  buffer += sizeof(double);
  return f;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryReadBuffer::readBuffer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxMemoryReadBuffer::readBuffer(void *dest, NxU32 size) const
{
  memcpy(dest, buffer, size);
  buffer += size;
}

