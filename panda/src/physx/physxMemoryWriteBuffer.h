// Filename: physxMemoryWriteBuffer.h
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

#ifndef PHYSXMEMORYWRITEBUFFER_H
#define PHYSXMEMORYWRITEBUFFER_H

#include "pandabase.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxMemoryWriteBuffer
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxMemoryWriteBuffer : public NxStream {

public:
  PhysxMemoryWriteBuffer();
  virtual ~PhysxMemoryWriteBuffer();

  virtual NxU8 readByte() const { NX_ASSERT(0); return 0; }
  virtual NxU16 readWord() const { NX_ASSERT(0); return 0; }
  virtual NxU32 readDword() const { NX_ASSERT(0); return 0; }
  virtual float readFloat() const { NX_ASSERT(0); return 0.0f; }
  virtual double readDouble() const { NX_ASSERT(0); return 0.0; }
  virtual void readBuffer(void *buffer, NxU32 size) const { NX_ASSERT(0); }

  virtual NxStream &storeByte(NxU8 b);
  virtual NxStream &storeWord(NxU16 w);
  virtual NxStream &storeDword(NxU32 d);
  virtual NxStream &storeFloat(NxReal f);
  virtual NxStream &storeDouble(NxF64 f);
  virtual NxStream &storeBuffer(const void *buffer, NxU32 size);

  NxU32 currentSize;
  NxU32 maxSize;
  NxU8 *data;
};

#endif // PHYSXMEMORYWRITEBUFFER_H
