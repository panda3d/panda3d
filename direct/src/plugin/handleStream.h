// Filename: handleStream.h
// Created by:  drose (05Jun09)
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

#ifndef HANDLESTREAM_H
#define HANDLESTREAM_H

#include "handleStreamBuf.h"

////////////////////////////////////////////////////////////////////
//       Class : HandleStream
// Description : Implements a C++ stream object suitable for reading
//               from and writing to Windows' HANDLE objects, or Posix
//               file descriptors.  This is necessary to map low-level
//               pipes into an iostream for tinyxml.
////////////////////////////////////////////////////////////////////
class HandleStream : public iostream {
public:
  inline HandleStream();
  inline ~HandleStream();

  inline void open_read(FHandle handle);
  inline void open_write(FHandle handle);
  inline void close();
  inline void close_handle();

  inline FHandle get_handle() const;
  inline bool has_gdata() const;

private:
  HandleStreamBuf _buf;
};

#include "handleStream.I"

#endif
