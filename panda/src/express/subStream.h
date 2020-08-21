/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subStream.h
 * @author drose
 * @date 2002-08-02
 */

#ifndef SUBSTREAM_H
#define SUBSTREAM_H

#include "pandabase.h"
#include "subStreamBuf.h"
#include "streamWrapper.h"

/**
 * An istream object that presents a subwindow into another istream.  The
 * first character read from this stream will be the "start" character from
 * the source istream; just before the file pointer reaches the "end"
 * character, eof is returned.
 *
 * The source stream must be one that we can randomly seek within.  The
 * resulting ISubStream will also support arbitrary seeks.
 */
class EXPCL_PANDA_EXPRESS ISubStream : public std::istream {
PUBLISHED:
  INLINE ISubStream();
  INLINE explicit ISubStream(IStreamWrapper *source, std::streampos start, std::streampos end);

#if _MSC_VER >= 1800
  INLINE ISubStream(const ISubStream &copy) = delete;
#endif

  INLINE ISubStream &open(IStreamWrapper *source, std::streampos start, std::streampos end);
  INLINE ISubStream &close();

private:
  SubStreamBuf _buf;
};

/**
 * An ostream object that presents a subwindow into another ostream.  The
 * first character written to this stream will be the "start" character in the
 * dest istream; no characters may be written to character "end" or later
 * (unless end is zero).
 *
 * The dest stream must be one that we can randomly seek within.  The
 * resulting OSubStream will also support arbitrary seeks.
 */
class EXPCL_PANDA_EXPRESS OSubStream : public std::ostream {
PUBLISHED:
  INLINE OSubStream();
  INLINE explicit OSubStream(OStreamWrapper *dest, std::streampos start, std::streampos end, bool append = false);

#if _MSC_VER >= 1800
  INLINE OSubStream(const OSubStream &copy) = delete;
#endif

  INLINE OSubStream &open(OStreamWrapper *dest, std::streampos start, std::streampos end, bool append = false);
  INLINE OSubStream &close();

private:
  SubStreamBuf _buf;
};

/**
 * Combined ISubStream and OSubStream for bidirectional I/O.
 */
class EXPCL_PANDA_EXPRESS SubStream : public std::iostream {
PUBLISHED:
  INLINE SubStream();
  INLINE explicit SubStream(StreamWrapper *nested, std::streampos start, std::streampos end, bool append = false);

#if _MSC_VER >= 1800
  INLINE SubStream(const SubStream &copy) = delete;
#endif

  INLINE SubStream &open(StreamWrapper *nested, std::streampos start, std::streampos end, bool append = false);
  INLINE SubStream &close();

private:
  SubStreamBuf _buf;
};

#include "subStream.I"

#endif
