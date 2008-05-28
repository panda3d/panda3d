// Filename: multiplexStreamBuf.h
// Created by:  drose (27Nov00)
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

#ifndef MULTIPLEXSTREAMBUF_H
#define MULTIPLEXSTREAMBUF_H

#include "pandabase.h"

#include "pvector.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////
//       Class : MultiplexStreamBuf
// Description : Used by MultiplexStream to implement an ostream that
//               sends what is written to it to any number of
//               additional sources, like other ostreams.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS MultiplexStreamBuf : public streambuf {
public:
  MultiplexStreamBuf();
  virtual ~MultiplexStreamBuf();

  enum BufferType {
    BT_none,
    BT_line,
  };

  enum OutputType {
    OT_ostream,
    OT_stdio,
    OT_system_debug,
  };

  void add_output(BufferType buffer_type, OutputType output_type,
                  ostream *out = (ostream *)NULL,
                  FILE *fout = (FILE *)NULL,
                  bool owns_obj = false);

  void flush();

protected:
  virtual int overflow(int c);
  virtual int sync();

private:
  void write_chars(const char *start, int length, bool flush);


  class Output {
  public:
    void close();
    void write_string(const string &str);

    BufferType _buffer_type;
    OutputType _output_type;
    ostream *_out;
    FILE *_fout;
    bool _owns_obj;
  };

  typedef pvector<Output> Outputs;
  Outputs _outputs;

  string _line_buffer;
};

#include "multiplexStreamBuf.I"

#endif
