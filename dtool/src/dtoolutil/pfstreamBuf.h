/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfstreamBuf.h
 * @author cary
 * @date 2000-12-12
 */

#ifndef __PFSTREAMBUF_H__
#define __PFSTREAMBUF_H__

#include "dtoolbase.h"
#include <string>
#include <stdio.h>

// By default, we'll use the Windows flavor of pipe functions if we're
// compiling under Windows.  Turn this off to use popen(), even on Windows.
// (popen() doesn't seem to work on Win9x, although it does work on NT-based
// variants.)
#ifdef WIN32_VC
#define WIN_PIPE_CALLS 1
#endif

#ifdef WIN_PIPE_CALLS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

#else  // WIN_PIPE_CALLS

#ifdef WIN32_VC
#define popen _popen
#define pclose _pclose
#endif

#endif // WIN_PIPE_CALLS

class EXPCL_DTOOL_DTOOLUTIL PipeStreamBuf : public std::streambuf {
public:
  enum Direction { Input, Output };

  PipeStreamBuf(Direction);
  virtual ~PipeStreamBuf(void);

  void flush();
  void command(const std::string);

protected:
  virtual int overflow(int c);
  virtual int sync(void);
  virtual int underflow(void);
private:
  void init_pipe();
  bool is_open() const;
  bool eof_pipe() const;
  bool open_pipe(const std::string &cmd);
  void close_pipe();
  size_t write_pipe(const char *data, size_t len);
  size_t read_pipe(char *data, size_t len);

  Direction _dir;
  std::string _line_buffer;

#ifndef WIN_PIPE_CALLS
  FILE *_pipe;

#else  // WIN_PIPE_CALLS
  HANDLE _child_out;
#endif  // WIN_PIPE_CALLS

  void write_chars(const char*, int, bool);
};

#endif /* __PFSTREAMBUF_H__ */
