// Filename: physxMemoryWriteBuffer.cxx
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

#include "physxMemoryWriteBuffer.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryWriteBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PhysxMemoryWriteBuffer::PhysxMemoryWriteBuffer() : currentSize(0), maxSize(0), data(NULL)
{

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryWriteBuffer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PhysxMemoryWriteBuffer::~PhysxMemoryWriteBuffer()
{
  NxGetPhysicsSDKAllocator()->free(data);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryWriteBuffer::storeByte
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxMemoryWriteBuffer::storeByte(NxU8 b)
{
  storeBuffer(&b, sizeof(NxU8));
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryWriteBuffer::storeWord
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxMemoryWriteBuffer::storeWord(NxU16 w)
{
  storeBuffer(&w, sizeof(NxU16));
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryWriteBuffer::storeDword
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxMemoryWriteBuffer::storeDword(NxU32 d)
{
  storeBuffer(&d, sizeof(NxU32));
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryWriteBuffer::storeFloat
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxMemoryWriteBuffer::storeFloat(NxReal f)
{
  storeBuffer(&f, sizeof(NxReal));
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryWriteBuffer::storeDouble
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxMemoryWriteBuffer::storeDouble(NxF64 f)
{
  storeBuffer(&f, sizeof(NxF64));
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMemoryWriteBuffer::storeBuffer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxMemoryWriteBuffer::storeBuffer(const void *buffer, NxU32 size)
{
  NxU32 expectedSize = currentSize + size;
  if (expectedSize > maxSize)
  {
    maxSize = expectedSize + 4096;

    NxU8 *newData = (NxU8 *)NxGetPhysicsSDKAllocator()->malloc(maxSize, NX_MEMORY_PERSISTENT);
    if(data)
    {
      memcpy(newData, data, currentSize);
      NxGetPhysicsSDKAllocator()->free(data);
    }
    data = newData;
  }
  memcpy(data + currentSize, buffer, size);
  currentSize += size;
  return *this;
}

