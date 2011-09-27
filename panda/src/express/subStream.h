// Filename: subStream.h
// Created by:  drose (02Aug02)
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

#ifndef SUBSTREAM_H
#define SUBSTREAM_H

#include "pandabase.h"
#include "subStreamBuf.h"
#include "streamWrapper.h"

////////////////////////////////////////////////////////////////////
//       Class : ISubStream
// Description : An istream object that presents a subwindow into
//               another istream.  The first character read from this
//               stream will be the "start" character from the source
//               istream; just before the file pointer reaches the
//               "end" character, eof is returned.
//
//               The source stream must be one that we can randomly
//               seek within.  The resulting ISubStream will also
//               support arbitrary seeks.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ISubStream : public istream {
PUBLISHED:
  INLINE ISubStream();
  INLINE ISubStream(IStreamWrapper *source, streampos start, streampos end);

  INLINE ISubStream &open(IStreamWrapper *source, streampos start, streampos end);
  INLINE ISubStream &close();

private:
  SubStreamBuf _buf;
};

////////////////////////////////////////////////////////////////////
//       Class : OSubStream
// Description : An ostream object that presents a subwindow into
//               another ostream.  The first character written to this
//               stream will be the "start" character in the dest
//               istream; no characters may be written to character
//               "end" or later (unless end is zero).
//
//               The dest stream must be one that we can randomly
//               seek within.  The resulting OSubStream will also
//               support arbitrary seeks.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS OSubStream : public ostream {
PUBLISHED:
  INLINE OSubStream();
  INLINE OSubStream(OStreamWrapper *dest, streampos start, streampos end, bool append = false);

  INLINE OSubStream &open(OStreamWrapper *dest, streampos start, streampos end, bool append = false);
  INLINE OSubStream &close();

private:
  SubStreamBuf _buf;
};

////////////////////////////////////////////////////////////////////
//       Class : SubStream
// Description : Combined ISubStream and OSubStream for bidirectional
//               I/O.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS SubStream : public iostream {
PUBLISHED:
  INLINE SubStream();
  INLINE SubStream(StreamWrapper *nested, streampos start, streampos end, bool append = false);

  INLINE SubStream &open(StreamWrapper *nested, streampos start, streampos end, bool append = false);
  INLINE SubStream &close();

private:
  SubStreamBuf _buf;
};

#include "subStream.I"

#endif


