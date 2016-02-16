/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMemoryReadBuffer.cxx
 * @author enn0x
 * @date 2009-10-11
 */

#include "physxMemoryReadBuffer.h"

/**

 */
PhysxMemoryReadBuffer::PhysxMemoryReadBuffer(const NxU8 *data) : buffer(data)
{

}

/**

 */
PhysxMemoryReadBuffer::~PhysxMemoryReadBuffer()
{

}

/**

 */
NxU8 PhysxMemoryReadBuffer::readByte() const
{
  NxU8 b;
  memcpy(&b, buffer, sizeof(NxU8));
  buffer += sizeof(NxU8);
  return b;
}

/**

 */
NxU16 PhysxMemoryReadBuffer::readWord() const
{
  NxU16 w;
  memcpy(&w, buffer, sizeof(NxU16));
  buffer += sizeof(NxU16);
  return w;
}

/**

 */
NxU32 PhysxMemoryReadBuffer::readDword() const
{
  NxU32 d;
  memcpy(&d, buffer, sizeof(NxU32));
  buffer += sizeof(NxU32);
  return d;
}

/**

 */
float PhysxMemoryReadBuffer::readFloat() const
{
  float f;
  memcpy(&f, buffer, sizeof(float));
  buffer += sizeof(float);
  return f;
}

/**

 */
double PhysxMemoryReadBuffer::readDouble() const
{
  double f;
  memcpy(&f, buffer, sizeof(double));
  buffer += sizeof(double);
  return f;
}

/**

 */
void PhysxMemoryReadBuffer::readBuffer(void *dest, NxU32 size) const
{
  memcpy(dest, buffer, size);
  buffer += size;
}
