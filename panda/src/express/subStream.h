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
public:
  INLINE ISubStream();
  INLINE ISubStream(IStreamWrapper *source, streampos start, streampos end);

  INLINE ISubStream &open(IStreamWrapper *source, streampos start, streampos end);
  INLINE ISubStream &close();

private:
  SubStreamBuf _buf;
};

#include "subStream.I"

#endif


