// Filename: pfstreamBuf.h
// Created by:  cary (12Dec00)
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

#ifndef __PFSTREAMBUF_H__
#define __PFSTREAMBUF_H__

#include <dtoolbase.h>
#include <string>
#include <stdio.h>

#ifdef WIN32_VC
#define popen _popen
#define pclose _pclose
#endif

class EXPCL_DTOOL PipeStreamBuf : public streambuf {
public:
  enum Direction { Input, Output };

  PipeStreamBuf(Direction);
  virtual ~PipeStreamBuf(void);

  void flush();
  void command(const string);
protected:
  virtual int overflow(int c);
  virtual int sync(void);
  virtual int underflow(void);
private:
  Direction _dir;
  string _line_buffer;
  FILE* _pipe;

  void write_chars(const char*, int, bool);
};

#endif /* __PFSTREAMBUF_H__ */
