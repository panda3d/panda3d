// Filename: pfstream.h
// Created by:  cary (27Aug98)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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


