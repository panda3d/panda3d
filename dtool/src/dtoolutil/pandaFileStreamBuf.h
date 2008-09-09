// Filename: pandaFileStreamBuf.h
// Created by:  drose (08Sep08)
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

#ifndef PANDAFILESTREAMBUF_H
#define PANDAFILESTREAMBUF_H

#include "dtoolbase.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

////////////////////////////////////////////////////////////////////
//       Class : PandaFileStreamBuf
// Description : The streambuf object that implements
//               pifstream and pofstream.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL PandaFileStreamBuf : public streambuf {
public:
  PandaFileStreamBuf();
  virtual ~PandaFileStreamBuf();

  void open(const char *filename, ios::openmode mode);
  bool is_open() const;
  void close();

protected:
  virtual streampos seekoff(streamoff off, ios_seekdir dir, ios_openmode which);
  virtual streampos seekpos(streampos pos, ios_openmode which);

  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  size_t read_chars(char *start, size_t length);
  void write_chars(const char *start, size_t length);

private:
  string _filename;
  bool _is_open;
  ios::openmode _open_mode;

#ifdef _WIN32
  HANDLE _handle;
#else
  int _fd;  // Posix file descriptor
#endif  // _WIN32

  char *_buffer;
  streampos _ppos;
  streampos _gpos;
};

#endif
