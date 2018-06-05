/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMemoryWriteBuffer.cxx
 * @author enn0x
 * @date 2009-10-11
 */

#include "physxMemoryWriteBuffer.h"

/**
 *
 */
PhysxMemoryWriteBuffer::PhysxMemoryWriteBuffer() : currentSize(0), maxSize(0), data(nullptr)
{

}

/**
 *
 */
PhysxMemoryWriteBuffer::~PhysxMemoryWriteBuffer()
{
  NxGetPhysicsSDKAllocator()->free(data);
}

/**
 *
 */
NxStream &PhysxMemoryWriteBuffer::storeByte(NxU8 b)
{
  storeBuffer(&b, sizeof(NxU8));
  return *this;
}

/**
 *
 */
NxStream &PhysxMemoryWriteBuffer::storeWord(NxU16 w)
{
  storeBuffer(&w, sizeof(NxU16));
  return *this;
}

/**
 *
 */
NxStream &PhysxMemoryWriteBuffer::storeDword(NxU32 d)
{
  storeBuffer(&d, sizeof(NxU32));
  return *this;
}

/**
 *
 */
NxStream &PhysxMemoryWriteBuffer::storeFloat(NxReal f)
{
  storeBuffer(&f, sizeof(NxReal));
  return *this;
}

/**
 *
 */
NxStream &PhysxMemoryWriteBuffer::storeDouble(NxF64 f)
{
  storeBuffer(&f, sizeof(NxF64));
  return *this;
}

/**
 *
 */
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
