// Filename: stringDecoder.h
// Created by:  drose (11Feb02)
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

#ifndef STRINGDECODER_H
#define STRINGDECODER_H

#include "pandabase.h"


////////////////////////////////////////////////////////////////////
//       Class : StringDecoder
// Description : The base class to a family of classes that decode
//               various kinds of encoded byte streams.  Give it a
//               string, then ask it to pull the characters out one at
//               a time.  This also serves as the plain old
//               byte-at-a-time decoder.
////////////////////////////////////////////////////////////////////
class StringDecoder {
public:
  INLINE StringDecoder(const string &input);
  virtual ~StringDecoder();

  virtual int get_next_character();
  INLINE bool is_eof();

protected:
  INLINE bool test_eof();

  string _input;
  size_t _p;
  bool _eof;
};

////////////////////////////////////////////////////////////////////
//       Class : StringUtf8Decoder
// Description : This decoder extracts utf-8 sequences.
////////////////////////////////////////////////////////////////////
class StringUtf8Decoder : public StringDecoder {
public:
  INLINE StringUtf8Decoder(const string &input);

  virtual int get_next_character();
};

////////////////////////////////////////////////////////////////////
//       Class : StringUnicodeDecoder
// Description : This decoder extracts characters two at a time to get
//               a plain wide character sequence.
////////////////////////////////////////////////////////////////////
class StringUnicodeDecoder : public StringDecoder {
public:
  INLINE StringUnicodeDecoder(const string &input);

  virtual int get_next_character();
};

#include "stringDecoder.I"

#endif
