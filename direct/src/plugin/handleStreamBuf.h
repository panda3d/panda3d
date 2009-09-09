// Filename: handleStreamBuf.h
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

#ifndef HANDLESTREAMBUF_H
#define HANDLESTREAMBUF_H

#include "fhandle.h"
#include "p3d_lock.h"
#include <iostream>

using namespace std;

////////////////////////////////////////////////////////////////////
//       Class : HandleStreamBuf
// Description : 
////////////////////////////////////////////////////////////////////
class HandleStreamBuf : public streambuf {
public:
  HandleStreamBuf();
  virtual ~HandleStreamBuf();

  void open_read(FHandle handle);
  void open_write(FHandle handle);
  bool is_open_read() const;
  bool is_open_write() const;
  void close();
  void close_handle();

  inline FHandle get_handle() const;
  inline bool has_gdata() const;

protected:
  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  size_t read_chars(char *start, size_t length);
  size_t write_chars(const char *start, size_t length);

private:
  bool _is_open_read;
  bool _is_open_write;

  FHandle _handle;
  LOCK _lock;

  char *_buffer;
};

#include "handleStreamBuf.I"

#endif
