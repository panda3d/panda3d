/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxFileStream.h
 * @author enn0x
 * @date 2009-10-11
 */

#ifndef PHYSXFILESTREAM_H
#define PHYSXFILESTREAM_H

#include "pandabase.h"
#include "virtualFile.h"
#include "filename.h"
#include <stdio.h>

#include "physx_includes.h"

/**
 *
 */
class EXPCL_PANDAPHYSX PhysxFileStream : public NxStream {

public:
  PhysxFileStream(const Filename &fn, bool load);
  virtual ~PhysxFileStream();

  virtual NxU8 readByte() const;
  virtual NxU16 readWord() const;
  virtual NxU32 readDword() const;
  virtual float readFloat() const;
  virtual double readDouble() const;
  virtual void readBuffer(void *buffer, NxU32 size) const;

  virtual NxStream &storeByte(NxU8 b);
  virtual NxStream &storeWord(NxU16 w);
  virtual NxStream &storeDword(NxU32 d);
  virtual NxStream &storeFloat(NxReal f);
  virtual NxStream &storeDouble(NxF64 f);
  virtual NxStream &storeBuffer(const void *buffer, NxU32 size);

private:

  // write
  FILE* _fp;

  // read
  PT(VirtualFile) _vf;
  std::istream *_in;
};

#endif // PHYSXFILESTREAM_H
