// Filename: multiplexStream.h
// Created by:  drose (27Nov00)
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

#ifndef MULTIPLEXSTREAM_H
#define MULTIPLEXSTREAM_H

#include "pandabase.h"

#include "multiplexStreamBuf.h"

#include <filename.h>

#include <stdio.h>

////////////////////////////////////////////////////////////////////
//       Class : MultiplexStream
// Description : This is a special ostream that forwards the data that
//               is written to it to any number of other sources, for
//               instance other ostreams, or explicitly to a disk file
//               or to system logging utilities.  It's a very handy
//               thing to set Notify to refer to when running in batch
//               mode.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS MultiplexStream : public ostream {
PUBLISHED:
  INLINE MultiplexStream();

  INLINE void add_ostream(ostream *out, bool delete_later = false);
  INLINE bool add_stdio_file(FILE *file, bool close_when_done);
  INLINE void add_standard_output();
  INLINE bool add_file(Filename file);
  INLINE void add_system_debug();

  INLINE void flush();

private:
  MultiplexStreamBuf _msb;
};

#include "multiplexStream.I"

#endif
