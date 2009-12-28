// Filename: physxFileStream.cxx
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

#include "physxFileStream.h"

#include "stdio.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PhysxFileStream::PhysxFileStream(const char *filename, bool load) : fp(NULL)
{
  fp = fopen(filename, load ? "rb" : "wb");
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PhysxFileStream::~PhysxFileStream()
{
  if (fp) fclose(fp);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::readByte
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxU8 PhysxFileStream::readByte() const
{
  NxU8 b;
  size_t r = fread(&b, sizeof(NxU8), 1, fp);
  NX_ASSERT(r);
  return b;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::readWord
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxU16 PhysxFileStream::readWord() const
{
  NxU16 w;
  size_t r = fread(&w, sizeof(NxU16), 1, fp);
  NX_ASSERT(r);
  return w;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::readDword
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxU32 PhysxFileStream::readDword() const
{
  NxU32 d;
  size_t r = fread(&d, sizeof(NxU32), 1, fp);
  NX_ASSERT(r);
  return d;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::readFloat
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
float PhysxFileStream::readFloat() const
{
  NxReal f;
  size_t r = fread(&f, sizeof(NxReal), 1, fp);
  NX_ASSERT(r);
  return f;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::readDouble
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
double PhysxFileStream::readDouble() const
{
  NxF64 f;
  size_t r = fread(&f, sizeof(NxF64), 1, fp);
  NX_ASSERT(r);
  return f;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::readBuffer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxFileStream::readBuffer(void *buffer, NxU32 size) const
{
  size_t w = fread(buffer, size, 1, fp);
  NX_ASSERT(w);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::storeByte
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxFileStream::storeByte(NxU8 b)
{
  size_t w = fwrite(&b, sizeof(NxU8), 1, fp);
  NX_ASSERT(w);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::storeWord
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxFileStream::storeWord(NxU16 w)
{
  size_t ww = fwrite(&w, sizeof(NxU16), 1, fp);
  NX_ASSERT(ww);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::storeDword
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxFileStream::storeDword(NxU32 d)
{
  size_t w = fwrite(&d, sizeof(NxU32), 1, fp);
  NX_ASSERT(w);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::storeFloat
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxFileStream::storeFloat(NxReal f)
{
  size_t w = fwrite(&f, sizeof(NxReal), 1, fp);
  NX_ASSERT(w);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::storeDouble
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxFileStream::storeDouble(NxF64 f)
{
  size_t w = fwrite(&f, sizeof(NxF64), 1, fp);
  NX_ASSERT(w);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFileStream::storeBuffer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxStream &PhysxFileStream::storeBuffer(const void *buffer, NxU32 size)
{
  size_t w = fwrite(buffer, size, 1, fp);
  NX_ASSERT(w);
  return *this;
}

