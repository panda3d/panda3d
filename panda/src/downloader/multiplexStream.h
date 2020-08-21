/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file multiplexStream.h
 * @author drose
 * @date 2000-11-27
 */

#ifndef MULTIPLEXSTREAM_H
#define MULTIPLEXSTREAM_H

#include "pandabase.h"

#include "multiplexStreamBuf.h"

#include "filename.h"

#include <stdio.h>

/**
 * This is a special ostream that forwards the data that is written to it to
 * any number of other sources, for instance other ostreams, or explicitly to
 * a disk file or to system logging utilities.  It's a very handy thing to set
 * Notify to refer to when running in batch mode.
 */
class EXPCL_PANDA_DOWNLOADER MultiplexStream : public std::ostream {
PUBLISHED:
  INLINE MultiplexStream();

#if _MSC_VER >= 1800
  INLINE MultiplexStream(const MultiplexStream &copy) = delete;
#endif

  INLINE void add_ostream(std::ostream *out, bool delete_later = false);
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
