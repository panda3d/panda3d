// Filename: pfstream.h
// Created by:  cary (27Aug98)
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

#ifndef __PFSTREAM_H__
#define __PFSTREAM_H__

#include "pfstreamBuf.h"

class EXPCL_DTOOL IPipeStream : public istream {
PUBLISHED:
  INLINE IPipeStream(const std::string);

  INLINE void flush();

private:
  PipeStreamBuf _psb;

  INLINE IPipeStream();
};

class EXPCL_DTOOL OPipeStream : public ostream {
PUBLISHED:
  INLINE OPipeStream(const std::string);

  INLINE void flush();

private:
  PipeStreamBuf _psb;

  INLINE OPipeStream();
};

#include "pfstream.I"

#endif /* __PFSTREAM_H__ */


