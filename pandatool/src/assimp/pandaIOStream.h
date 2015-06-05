// Filename: pandaIOStream.h
// Created by:  rdb (29Mar11)
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

#ifndef PANDAIOSTREAM_H
#define PANDAIOSTREAM_H

#include "config_assimp.h"

#include "assimp/IOStream.hpp"

class PandaIOSystem;

////////////////////////////////////////////////////////////////////
//       Class : PandaIOStream
// Description : Custom implementation of Assimp::IOStream.  It
//               simply wraps around an istream object, and is
//               unable to write.
////////////////////////////////////////////////////////////////////
class PandaIOStream : public Assimp::IOStream {
public:
  PandaIOStream(istream &stream);
  virtual ~PandaIOStream() {};

  size_t FileSize() const;
  void Flush();
  size_t Read(void *pvBuffer, size_t pSize, size_t pCount);
  aiReturn Seek(size_t pOffset, aiOrigin pOrigin);
  size_t Tell() const;
  size_t Write(const void *buffer, size_t size, size_t count);

private:
  istream &_istream;

  friend class PandaIOSystem;
};

#endif
