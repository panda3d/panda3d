// Filename: physxMemoryReadBuffer.h
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

#ifndef PHYSXMEMORYREADBUFFER_H
#define PHYSXMEMORYREADBUFFER_H

#include "pandabase.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxMemoryReadBuffer
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxMemoryReadBuffer : public NxStream {

public:
  PhysxMemoryReadBuffer(const NxU8 *data);
  virtual ~PhysxMemoryReadBuffer();

  virtual NxU8 readByte() const;
  virtual NxU16 readWord() const;
  virtual NxU32 readDword() const;
  virtual float readFloat() const;
  virtual double readDouble() const;
  virtual void readBuffer(void *buffer, NxU32 size) const;

  virtual NxStream &storeByte(NxU8 b) { NX_ASSERT(0); return *this; }
  virtual NxStream &storeWord(NxU16 w) { NX_ASSERT(0); return *this; }
  virtual NxStream &storeDword(NxU32 d) { NX_ASSERT(0); return *this; }
  virtual NxStream &storeFloat(NxReal f) { NX_ASSERT(0); return *this; }
  virtual NxStream &storeDouble(NxF64 f) { NX_ASSERT(0); return *this; }
  virtual NxStream &storeBuffer(const void *buffer, NxU32 size) { NX_ASSERT(0); return *this; }

  mutable const NxU8 *buffer;
};

#endif // PHYSXMEMORYREADBUFFER_H
