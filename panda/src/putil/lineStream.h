// Filename: lineStream.h
// Created by:  drose (26Feb00)
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

#ifndef LINESTREAM_H
#define LINESTREAM_H

#include "pandabase.h"

#include "lineStreamBuf.h"

////////////////////////////////////////////////////////////////////
//       Class : LineStream
// Description : This is a special ostream that writes to a memory
//               buffer, like ostrstream.  However, its contents can
//               be continuously extracted as a sequence of lines of
//               text.
//
//               Unlike ostrstream, which can only be extracted from
//               once (and then the buffer freezes and it can no
//               longer be written to), the LineStream is not
//               otherwise affected when a line of text is extracted.
//               More text can still be written to it and continuously
//               extracted.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LineStream : public ostream {
PUBLISHED:
  INLINE LineStream();

  INLINE bool is_text_available() const;
  INLINE string get_line();
  INLINE bool has_newline() const;

private:
  LineStreamBuf _lsb;
};

#include "lineStream.I"

#endif
